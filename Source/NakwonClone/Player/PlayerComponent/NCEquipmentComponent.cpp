#include "NCEquipmentComponent.h"
#include "NCGunComponent.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"

UNCEquipmentComponent::UNCEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNCEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ─────────────────────────────────────────────
// 상태 조회

bool UNCEquipmentComponent::CanFire() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->CanFire();
	return false;
}

bool UNCEquipmentComponent::IsFiring() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->IsFiring();
	return false;
}

bool UNCEquipmentComponent::IsReloading() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->IsReloading();
	return false;
}

bool UNCEquipmentComponent::IsADS() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->IsADS();
	return false;
}

int32 UNCEquipmentComponent::GetCurrentAmmo() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->CurrentAmmo;
	return 0;
}

int32 UNCEquipmentComponent::GetReserveAmmo() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->ReserveAmmo;
	return 0;
}

FName UNCEquipmentComponent::GetActiveGunID() const
{
	if (ActiveSlot == ENCGunSlot::Primary)   return PrimarySlot.GunID;
	if (ActiveSlot == ENCGunSlot::Secondary) return SecondarySlot.GunID;
	return NAME_None;
}

FName UNCEquipmentComponent::GetOccupantGunID(FName ForGunID) const
{
	const FNCGunData* Data = FindGunData(ForGunID);
	if (!Data) return NAME_None;
	if (Data->SlotType == ENCGunSlot::Primary)   return PrimarySlot.GunID;
	if (Data->SlotType == ENCGunSlot::Secondary) return SecondarySlot.GunID;
	return NAME_None;
}

// ─────────────────────────────────────────────
// 무기 장착 / 해제

bool UNCEquipmentComponent::EquipGun(FName GunID)
{
	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	if (ActiveSlot == Data->SlotType)
		DeactivateCurrentWeapon();

	FNCGunSlotData& Slot = GetSlotData(Data->SlotType);
	Slot.GunID       = GunID;
	Slot.GunTypeTag  = Data->GunTypeTag;
	Slot.CurrentAmmo = Data->MagazineSize;
	Slot.ReserveAmmo = Data->MaxReserveAmmo;

	if (ActiveSlot == Data->SlotType)
		ActivateWeaponForSlot(Data->SlotType);

	OnGunEquipped.Broadcast(GunID);
	return true;
}

bool UNCEquipmentComponent::EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo)
{
	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	if (ActiveSlot == Data->SlotType)
		DeactivateCurrentWeapon();

	FNCGunSlotData& Slot = GetSlotData(Data->SlotType);
	Slot.GunID       = GunID;
	Slot.GunTypeTag  = Data->GunTypeTag;
	Slot.CurrentAmmo = FMath::Clamp(CurrentAmmo, 0, Data->MagazineSize);
	Slot.ReserveAmmo = FMath::Clamp(ReserveAmmo, 0, Data->MaxReserveAmmo);

	if (ActiveSlot == Data->SlotType)
		ActivateWeaponForSlot(Data->SlotType);

	OnGunEquipped.Broadcast(GunID);
	return true;
}

void UNCEquipmentComponent::UnequipGun(ENCGunSlot Slot)
{
	if (ActiveSlot == Slot)
	{
		GetWorld()->GetTimerManager().ClearTimer(SwapTimerHandle);
		bIsSwapping = false;
		DeactivateCurrentWeapon();
		ActiveSlot = ENCGunSlot::None;
		OnGunUnequipped.Broadcast();
	}

	GetSlotData(Slot).Clear();
}

void UNCEquipmentComponent::SelectSlot(ENCGunSlot Slot)
{
	if (ActiveSlot == Slot) return;
	if (bIsSwapping) return;

	// 현재 무기 정지 및 비활성화
	if (UNCGunComponent* Current = GetActiveWeapon())
	{
		Current->StopFire();
		Current->StopADS();
		if (const FNCGunData* Data = Current->GetActiveGunData())
			Current->PlayUnequipMontage(Data);
	}

	bIsSwapping = true;
	DeactivateCurrentWeapon();

	GetWorld()->GetTimerManager().SetTimer(
		SwapTimerHandle,
		FTimerDelegate::CreateUObject(this, &UNCEquipmentComponent::OnSwapFinished, Slot),
		SwapDelay, false);
}

void UNCEquipmentComponent::OnSwapFinished(ENCGunSlot TargetSlot)
{
	bIsSwapping = false;
	ActiveSlot  = TargetSlot;

	if (TargetSlot != ENCGunSlot::None)
		ActivateWeaponForSlot(TargetSlot);
	else
		OnGunUnequipped.Broadcast();

	OnSwapCompleted.Broadcast(TargetSlot);
}

// ─────────────────────────────────────────────
// 액션 프록시

void UNCEquipmentComponent::StartFire()
{
	if (UNCGunComponent* W = GetActiveWeapon()) W->StartFire();
}

void UNCEquipmentComponent::StopFire()
{
	if (UNCGunComponent* W = GetActiveWeapon()) W->StopFire();
}

void UNCEquipmentComponent::Reload()
{
	if (UNCGunComponent* W = GetActiveWeapon()) W->Reload();
}

void UNCEquipmentComponent::StartADS()
{
	if (!HasActiveGun() || IsSwapping()) return;
	if (UNCGunComponent* W = GetActiveWeapon()) W->StartADS();
}

void UNCEquipmentComponent::StopADS()
{
	if (UNCGunComponent* W = GetActiveWeapon()) W->StopADS();
}

void UNCEquipmentComponent::ToggleFireMode()
{
	if (UNCGunComponent* W = GetActiveWeapon()) W->ToggleFireMode();
}

// ─────────────────────────────────────────────
// 데이터 접근

const FNCGunData* UNCEquipmentComponent::GetActiveGunData() const
{
	if (UNCGunComponent* W = GetActiveWeapon()) return W->GetActiveGunData();
	return nullptr;
}

UNCGunComponent* UNCEquipmentComponent::GetActiveWeapon() const
{
	const FNCGunSlotData& Slot = (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	if (ActiveSlot == ENCGunSlot::None || !Slot.GunTypeTag.IsValid()) return nullptr;

	if (const TObjectPtr<UNCGunComponent>* Found = WeaponComponents.Find(Slot.GunTypeTag))
		return *Found;
	return nullptr;
}

// ─────────────────────────────────────────────
// 내부 헬퍼

void UNCEquipmentComponent::ActivateWeaponForSlot(ENCGunSlot Slot)
{
	FNCGunSlotData& SlotData = GetSlotData(Slot);
	const FNCGunData* Data   = FindGunData(SlotData.GunID);
	if (!Data) return;

	const TObjectPtr<UNCGunComponent>* Found = WeaponComponents.Find(SlotData.GunTypeTag);
	if (!Found || !*Found) return;
	UNCGunComponent* Weapon = *Found;

	// 무기 활성화
	Weapon->ActivateGun(Data, SlotData.CurrentAmmo, SlotData.ReserveAmmo);

	// 이벤트 구독 (중복 방지)
	Weapon->OnAmmoChanged.RemoveDynamic(this, &UNCEquipmentComponent::OnActiveWeaponAmmoChanged);
	Weapon->OnAmmoChanged.AddDynamic(this, &UNCEquipmentComponent::OnActiveWeaponAmmoChanged);
	Weapon->OnFireModeChanged.RemoveDynamic(this, &UNCEquipmentComponent::OnActiveWeaponFireModeChanged);
	Weapon->OnFireModeChanged.AddDynamic(this, &UNCEquipmentComponent::OnActiveWeaponFireModeChanged);

	// UI에 초기 탄약 알림
	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
}

void UNCEquipmentComponent::DeactivateCurrentWeapon()
{
	UNCGunComponent* Weapon = GetActiveWeapon();
	if (!Weapon) return;

	// 탄약 상태 저장
	FNCGunSlotData& SlotData = GetSlotData(ActiveSlot);
	SlotData.CurrentAmmo = Weapon->CurrentAmmo;
	SlotData.ReserveAmmo = Weapon->ReserveAmmo;

	// 이벤트 구독 해제
	Weapon->OnAmmoChanged.RemoveDynamic(this, &UNCEquipmentComponent::OnActiveWeaponAmmoChanged);
	Weapon->OnFireModeChanged.RemoveDynamic(this, &UNCEquipmentComponent::OnActiveWeaponFireModeChanged);

	Weapon->DeactivateGun();
}

const FNCGunData* UNCEquipmentComponent::FindGunData(FName GunID) const
{
	if (!GunDataTable || GunID.IsNone()) return nullptr;
	return GunDataTable->FindRow<FNCGunData>(GunID, TEXT("NCEquipmentComponent"));
}

FNCGunSlotData& UNCEquipmentComponent::GetSlotData(ENCGunSlot Slot)
{
	return (Slot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
}

void UNCEquipmentComponent::OnActiveWeaponAmmoChanged(int32 CurrentAmmo, int32 ReserveAmmo)
{
	OnAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo);
}

void UNCEquipmentComponent::OnActiveWeaponFireModeChanged(ENCFireMode NewFireMode)
{
	OnFireModeChanged.Broadcast(NewFireMode);
}
