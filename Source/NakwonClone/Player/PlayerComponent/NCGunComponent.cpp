#include "NCGunComponent.h"
#include "Common/NCGameplayTags.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Gun/NCProjectile.h"

UNCGunComponent::UNCGunComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNCGunComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UCameraComponent* Cam = FindCamera())
		DefaultFOV = Cam->FieldOfView;
}

// ─────────────────────────────────────────────
// 상태 조회

bool UNCGunComponent::IsFiring() const    { return ActiveGunActions.HasTag(NCGun::Action_Firing); }
bool UNCGunComponent::IsReloading() const { return ActiveGunActions.HasTag(NCGun::Action_Reloading); }
bool UNCGunComponent::IsADS() const       { return ActiveGunActions.HasTag(NCGun::Action_ADS); }
bool UNCGunComponent::IsSwapping() const  { return ActiveGunActions.HasTag(NCGun::Action_Swapping); }

bool UNCGunComponent::CanFire() const
{
	if (!HasActiveGun()) return false;
	if (IsReloading())   return false;
	if (IsSwapping())    return false;

	const FNCGunSlotData& SlotData = (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	return SlotData.CurrentAmmo > 0;
}
// ─────────────────────────────────────────────
// 탄약 / 총기 정보 getter

int32 UNCGunComponent::GetCurrentAmmo() const
{
	if (!HasActiveGun()) return 0;
	return (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot.CurrentAmmo : SecondarySlot.CurrentAmmo;
}

int32 UNCGunComponent::GetReserveAmmo() const
{
	if (!HasActiveGun()) return 0;
	return (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot.ReserveAmmo : SecondarySlot.ReserveAmmo;
}

FName UNCGunComponent::GetActiveGunID() const
{
	if (ActiveSlot == ENCGunSlot::Primary)   return PrimarySlot.GunID;
	if (ActiveSlot == ENCGunSlot::Secondary) return SecondarySlot.GunID;
	return NAME_None;
}

// ─────────────────────────────────────────────
// 장착 / 해제

bool UNCGunComponent::EquipGun(FName GunID)
{
	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	FNCGunSlotData& Slot = (Data->SlotType == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	Slot.GunID       = GunID;
	Slot.CurrentAmmo = Data->MagazineSize;
	Slot.ReserveAmmo = Data->MaxReserveAmmo;

	if (ActiveSlot == Data->SlotType)
	{
		CurrentFireMode = Data->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);
	}

	OnGunEquipped.Broadcast(GunID);
	return true;
}

bool UNCGunComponent::EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo)
{
	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	FNCGunSlotData& Slot = (Data->SlotType == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	Slot.GunID       = GunID;
	Slot.CurrentAmmo = FMath::Clamp(CurrentAmmo, 0, Data->MagazineSize);
	Slot.ReserveAmmo = FMath::Clamp(ReserveAmmo, 0, Data->MaxReserveAmmo);

	if (ActiveSlot == Data->SlotType)
	{
		CurrentFireMode = Data->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);
	}

	OnGunEquipped.Broadcast(GunID);
	return true;
}

void UNCGunComponent::UnequipGun(ENCGunSlot Slot)
{
	if (Slot == ENCGunSlot::Primary)   PrimarySlot.Clear();
	if (Slot == ENCGunSlot::Secondary) SecondarySlot.Clear();

	if (ActiveSlot == Slot)
	{
		GetWorld()->GetTimerManager().ClearTimer(FullAutoTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(SwapTimerHandle);
		ActiveSlot = ENCGunSlot::None;
		ActiveGunActions.Reset();
		RestoreFOV();
		OnGunUnequipped.Broadcast();
	}
}

void UNCGunComponent::SelectSlot(ENCGunSlot Slot)
{
	if (ActiveSlot == Slot) return;
	if (IsSwapping()) return;

	StopFire();
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	RestoreFOV();

	// 태그 정리 후 스왑
	ActiveGunActions.Reset();
	ActiveGunActions.AddTag(NCGun::Action_Swapping);

	// SwapDelay 후 실제 슬롯 전환
	GetWorld()->GetTimerManager().SetTimer(
		SwapTimerHandle,
		FTimerDelegate::CreateUObject(this, &UNCGunComponent::OnSwapFinished, Slot),
		SwapDelay, false);
}

void UNCGunComponent::OnSwapFinished(ENCGunSlot TargetSlot)
{
	ActiveGunActions.RemoveTag(NCGun::Action_Swapping);
	ActiveSlot = TargetSlot;

	const FNCGunData* Data = GetActiveGunData();
	if (Data)
	{
		CurrentFireMode = Data->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);

		const FNCGunSlotData& SlotData = GetActiveSlotData();
		OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
		OnGunEquipped.Broadcast(SlotData.GunID);
	}
	else
	{
		OnGunUnequipped.Broadcast();
	}
}

// ─────────────────────────────────────────────
// 사격

void UNCGunComponent::StartFire()
{
	// 탄약 없을 때 빈 총 클릭음
	if (HasActiveGun() && !IsReloading() && !IsSwapping())
	{
		const FNCGunSlotData& SlotData = (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
		if (SlotData.CurrentAmmo <= 0)
		{
			const FNCGunData* Data = GetActiveGunData();
			if (Data && Data->EmptyClickSound.IsValid())
				UGameplayStatics::PlaySound2D(this, Data->EmptyClickSound.Get());
			return;
		}
	}

	if (!CanFire()) return;

	ActiveGunActions.AddTag(NCGun::Action_Firing);
	FireOnce();

	if (CurrentFireMode == ENCFireMode::FullAuto)
	{
		const FNCGunData* Data = GetActiveGunData();
		if (!Data) return;

		const float Interval = (Data->FireRate > 0.f) ? (1.f / Data->FireRate) : 0.1f;
		GetWorld()->GetTimerManager().SetTimer(
			FullAutoTimerHandle,
			this, &UNCGunComponent::FireOnce,
			Interval, true);
	}
	else
	{
		ActiveGunActions.RemoveTag(NCGun::Action_Firing);
	}
}

void UNCGunComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FullAutoTimerHandle);
	ActiveGunActions.RemoveTag(NCGun::Action_Firing);
}

void UNCGunComponent::FireOnce()
{
	if (!CanFire())
	{
		StopFire();
		return;
	}

	const FNCGunData* Data = GetActiveGunData();
	if (!Data || !Data->ProjectileClass) return;

	FNCGunSlotData& SlotData = GetActiveSlotData();
	--SlotData.CurrentAmmo;
	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector  SpawnLocation = Owner->GetActorLocation();
	FRotator SpawnRotation = Owner->GetActorRotation();

	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		if (UCameraComponent* Cam = Char->FindComponentByClass<UCameraComponent>())
		{
			SpawnLocation = Cam->GetComponentLocation();
			SpawnRotation = Cam->GetComponentRotation();
		}
	}

	const int32 PelletCount = FMath::Max(1, Data->NumPellets);
	for (int32 i = 0; i < PelletCount; ++i)
	{
		FRotator PelletRotation = SpawnRotation;
		if (Data->SpreadAngle > 0.f)
		{
			PelletRotation.Yaw   += FMath::RandRange(-Data->SpreadAngle, Data->SpreadAngle);
			PelletRotation.Pitch += FMath::RandRange(-Data->SpreadAngle, Data->SpreadAngle);
		}

		FActorSpawnParameters Params;
		Params.Owner      = Owner;
		Params.Instigator = Cast<APawn>(Owner);
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ANCProjectile* NCProj = GetWorld()->SpawnActor<ANCProjectile>(Data->ProjectileClass, SpawnLocation, PelletRotation, Params))
		{
			NCProj->Damage          = Data->Damage;
			NCProj->MaxRange        = Data->MaxRange;
			NCProj->ProjectileSpeed = Data->ProjectileSpeed;
			NCProj->ImpactFleshEffect   = Data->ImpactFleshEffect.Get();
			NCProj->ImpactSurfaceEffect = Data->ImpactSurfaceEffect.Get();
		}
	}
}
// ─────────────────────────────────────────────
// 재장전

void UNCGunComponent::Reload()
{
	if (IsReloading() || IsSwapping() || !HasActiveGun()) return;

	FNCGunSlotData& SlotData = GetActiveSlotData();
	const FNCGunData* Data   = GetActiveGunData();
	if (!Data) return;

	if (SlotData.ReserveAmmo <= 0)                  return;
	if (SlotData.CurrentAmmo >= Data->MagazineSize) return;

	StopFire();
	ActiveGunActions.AddTag(NCGun::Action_Reloading);

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this, &UNCGunComponent::OnReloadFinished,
		Data->ReloadTime, false);
}

void UNCGunComponent::OnReloadFinished()
{
	ActiveGunActions.RemoveTag(NCGun::Action_Reloading);

	FNCGunSlotData& SlotData = GetActiveSlotData();
	const FNCGunData* Data   = GetActiveGunData();
	if (!Data) return;

	const int32 Needed = Data->MagazineSize - SlotData.CurrentAmmo;
	const int32 Take   = FMath::Min(Needed, SlotData.ReserveAmmo);
	SlotData.CurrentAmmo += Take;
	SlotData.ReserveAmmo -= Take;

	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
}

// ─────────────────────────────────────────────
// ADS

void UNCGunComponent::StartADS()
{
	if (!HasActiveGun() || IsSwapping()) return;
	ActiveGunActions.AddTag(NCGun::Action_ADS);
	ApplyADSFOV();
}

void UNCGunComponent::StopADS()
{
	ActiveGunActions.RemoveTag(NCGun::Action_ADS);
	RestoreFOV();
}

void UNCGunComponent::ApplyADSFOV()
{
	const FNCGunData* Data = GetActiveGunData();
	if (!Data) return;

	if (UCameraComponent* Cam = FindCamera())
		Cam->SetFieldOfView(DefaultFOV * Data->ADSFOVMultiplier);
}

void UNCGunComponent::RestoreFOV()
{
	if (UCameraComponent* Cam = FindCamera())
		Cam->SetFieldOfView(DefaultFOV);
}

UCameraComponent* UNCGunComponent::FindCamera() const
{
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		return Char->FindComponentByClass<UCameraComponent>();
	return nullptr;
}

// ─────────────────────────────────────────────
// 발사 모드 전환

void UNCGunComponent::ToggleFireMode()
{
	const FNCGunData* Data = GetActiveGunData();
	if (!Data || !Data->bCanToggleFireMode) return;

	ActiveGunActions.AddTag(NCGun::Action_ToggleFireMode);

	CurrentFireMode = (CurrentFireMode == ENCFireMode::SemiAuto)
		? ENCFireMode::FullAuto
		: ENCFireMode::SemiAuto;

	OnFireModeChanged.Broadcast(CurrentFireMode);

	ActiveGunActions.RemoveTag(NCGun::Action_ToggleFireMode);
}

// ─────────────────────────────────────────────
// 헬퍼

const FNCGunData* UNCGunComponent::GetActiveGunData() const
{
	if (ActiveSlot == ENCGunSlot::Primary)   return FindGunData(PrimarySlot.GunID);
	if (ActiveSlot == ENCGunSlot::Secondary) return FindGunData(SecondarySlot.GunID);
	return nullptr;
}

FNCGunSlotData& UNCGunComponent::GetActiveSlotData()
{
	return (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
}

const FNCGunData* UNCGunComponent::FindGunData(FName GunID) const
{
	if (!GunDataTable || GunID.IsNone()) return nullptr;
	return GunDataTable->FindRow<FNCGunData>(GunID, TEXT("NCGunComponent"));
}
