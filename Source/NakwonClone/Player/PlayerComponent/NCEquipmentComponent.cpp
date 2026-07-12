#include "NCEquipmentComponent.h"
#include "NCGunComponent.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "Weapon/Gun/NCGunActor.h"

UNCEquipmentComponent::UNCEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNCEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNCEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SwapTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(EquipDelayTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// ─────────────────────────────────────────────
// 상태 조회

bool UNCEquipmentComponent::CanFire() const
{
	if (bIsSwapping) return false;

	if (UNCGunComponent* W = GetActiveWeapon())
	{
		return W->CanFire();
	}

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
	switch (ActiveSlot)
	{
	case ENCGunSlot::Shotgun:
		return ShotgunSlot.GunID;
	case ENCGunSlot::Rifle:
		return RifleSlot.GunID;
	case ENCGunSlot::Sidearm:
		return SidearmSlot.GunID;
	default:
		return NAME_None;
	}
}

UTexture2D* UNCEquipmentComponent::GetGunIcon(ENCGunSlot Slot) const
{
	const FNCGunSlotData* SlotData = nullptr;

	switch (Slot)
	{
	case ENCGunSlot::Shotgun:
		SlotData = &ShotgunSlot;
		break;
	case ENCGunSlot::Rifle:
		SlotData = &RifleSlot;
		break;
	case ENCGunSlot::Sidearm:
		SlotData = &SidearmSlot;
		break;
	default:
		return nullptr;
	}

	if (!SlotData || SlotData->GunID.IsNone()) return nullptr;

	const FNCGunData* Data = FindGunData(SlotData->GunID);
	return Data ? Data->Icon : nullptr;
}

FName UNCEquipmentComponent::GetOccupantGunID(FName ForGunID) const
{
	const FNCGunData* Data = FindGunData(ForGunID);
	if (!Data) return NAME_None;

	switch (Data->SlotType)
	{
	case ENCGunSlot::Shotgun:
		return ShotgunSlot.GunID;
	case ENCGunSlot::Rifle:
		return RifleSlot.GunID;
	case ENCGunSlot::Sidearm:
		return SidearmSlot.GunID;
	default:
		return NAME_None;
	}
}

// ─────────────────────────────────────────────
// 무기 장착 / 해제

bool UNCEquipmentComponent::EquipGun(FName GunID)
{
	if (bIsSwapping) return false;

	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	if (ActiveSlot == Data->SlotType)
	{
		DeactivateCurrentWeapon();
	}

	FNCGunSlotData& Slot = GetSlotData(Data->SlotType);
	Slot.GunID = GunID;
	Slot.GunTypeTag = Data->GunTypeTag;
	Slot.CurrentAmmo = Data->MagazineSize;
	// Slot.ReserveAmmo = Data->MaxReserveAmmo;

	if (ActiveSlot == Data->SlotType)
	{
		ActivateWeaponForSlot(Data->SlotType);
	}

	OnGunEquipped.Broadcast(GunID);
	OnGunSlotChanged.Broadcast(Data->SlotType, GunID);
	return true;
}

bool UNCEquipmentComponent::EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo)
{
	if (bIsSwapping) return false;

	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	if (ActiveSlot == Data->SlotType)
	{
		DeactivateCurrentWeapon();
	}

	FNCGunSlotData& Slot = GetSlotData(Data->SlotType);
	Slot.GunID = GunID;
	Slot.GunTypeTag = Data->GunTypeTag;
	Slot.CurrentAmmo = FMath::Clamp(CurrentAmmo, 0, Data->MagazineSize);
	// Slot.ReserveAmmo = FMath::Clamp(ReserveAmmo, 0, Data->MaxReserveAmmo);

	if (ActiveSlot == Data->SlotType)
	{
		ActivateWeaponForSlot(Data->SlotType);
	}

	OnGunEquipped.Broadcast(GunID);
	OnGunSlotChanged.Broadcast(Data->SlotType, GunID);
	return true;
}

void UNCEquipmentComponent::UnequipGun(ENCGunSlot Slot)
{
	GetWorld()->GetTimerManager().ClearTimer(SwapTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(EquipDelayTimerHandle);

	if (ActiveSlot == Slot)
	{
		bIsSwapping = false;
		PendingEquipSlot = ENCGunSlot::None;

		DeactivateCurrentWeapon();
		ActiveSlot = ENCGunSlot::None;

		OnGunUnequipped.Broadcast();
	}

	GetSlotData(Slot).Clear();
	OnGunSlotChanged.Broadcast(Slot, NAME_None);
}

void UNCEquipmentComponent::SelectSlot(ENCGunSlot Slot)
{
	if (ActiveSlot == Slot) return;

	if (bIsSwapping && PendingEquipSlot == Slot) return;

	if (Slot != ENCGunSlot::None)
	{
		FNCGunSlotData& TargetSlotData = GetSlotData(Slot);
		if (TargetSlotData.GunID.IsNone())
		{
			return;
		}
	}

	bIsSwapping = true;
	PendingEquipSlot = Slot;

	GetWorld()->GetTimerManager().ClearTimer(SwapTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(EquipDelayTimerHandle);

	float UnequipLength = 0.f;

	if (UNCGunComponent* Current = GetActiveWeapon())
	{
		Current->StopFire();

		if (Current->IsADS())
		{
			Current->StopADS();
		}

		if (const FNCGunData* Data = Current->GetActiveGunData())
		{
			UnequipLength = Current->PlayUnequipMontage(Data);
		}
	}

	if (UnequipLength > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			SwapTimerHandle,
			FTimerDelegate::CreateUObject(this, &UNCEquipmentComponent::OnSwapFinished, Slot),
			UnequipLength,
			false
		);
	}
	else
	{
		OnSwapFinished(Slot);
	}
}

void UNCEquipmentComponent::OnSwapFinished(ENCGunSlot TargetSlot)
{
	// Unequip 몽타주가 끝난 뒤 기존 무기 비활성화
	DeactivateCurrentWeapon();

	// 그 다음 슬롯 변경
	ActiveSlot = TargetSlot;

	if (TargetSlot == ENCGunSlot::None)
	{
		bIsSwapping = false;
		PendingEquipSlot = ENCGunSlot::None;

		OnGunUnequipped.Broadcast();
		OnSwapCompleted.Broadcast(TargetSlot);
		return;
	}

	// Unequip 끝난 직후 바로 다음 무기 Equip
	ActivatePendingWeapon();
}

void UNCEquipmentComponent::ActivatePendingWeapon()
{
	if (PendingEquipSlot == ENCGunSlot::None)
	{
		bIsSwapping = false;
		OnGunUnequipped.Broadcast();
		OnSwapCompleted.Broadcast(ENCGunSlot::None);
		return;
	}

	ActivateWeaponForSlot(PendingEquipSlot);

	bIsSwapping = false;

	OnSwapCompleted.Broadcast(PendingEquipSlot);

	PendingEquipSlot = ENCGunSlot::None;
}

// ─────────────────────────────────────────────
// 액션 프록시

void UNCEquipmentComponent::StartFire()
{
	if (bIsSwapping) return;

	if (UNCGunComponent* W = GetActiveWeapon())
	{
		W->StartFire();
	}
}

void UNCEquipmentComponent::StopFire()
{
	if (UNCGunComponent* W = GetActiveWeapon())
	{
		W->StopFire();
	}
}

void UNCEquipmentComponent::Reload()
{
	if (bIsSwapping) return;

	if (UNCGunComponent* W = GetActiveWeapon())
	{
		W->Reload();
	}
}

void UNCEquipmentComponent::StartADS()
{
	if (!HasActiveGun() || IsSwapping()) return;

	if (UNCGunComponent* W = GetActiveWeapon())
	{
		W->StartADS();
	}
}

void UNCEquipmentComponent::StopADS()
{
	if (UNCGunComponent* W = GetActiveWeapon())
	{
		W->StopADS();
	}
}

void UNCEquipmentComponent::ToggleFireMode()
{
	if (bIsSwapping) return;

	if (UNCGunComponent* W = GetActiveWeapon())
	{
		W->ToggleFireMode();
	}
}

// ─────────────────────────────────────────────
// 데이터 접근

const FNCGunData* UNCEquipmentComponent::GetActiveGunData() const
{
	if (UNCGunComponent* W = GetActiveWeapon())
	{
		return W->GetActiveGunData();
	}

	return nullptr;
}

UNCGunComponent* UNCEquipmentComponent::GetActiveWeapon() const
{
	if (ActiveSlot == ENCGunSlot::None)
	{
		return nullptr;
	}

	const FNCGunSlotData* Slot = nullptr;

	switch (ActiveSlot)
	{
	case ENCGunSlot::Shotgun:
		Slot = &ShotgunSlot;
		break;
	case ENCGunSlot::Rifle:
		Slot = &RifleSlot;
		break;
	case ENCGunSlot::Sidearm:
		Slot = &SidearmSlot;
		break;
	default:
		return nullptr;
	}

	if (!Slot || !Slot->GunTypeTag.IsValid())
	{
		return nullptr;
	}

	if (const TObjectPtr<UNCGunComponent>* Found = WeaponComponents.Find(Slot->GunTypeTag))
	{
		return *Found;
	}

	return nullptr;
}

// ─────────────────────────────────────────────
// 내부 헬퍼

void UNCEquipmentComponent::ActivateWeaponForSlot(ENCGunSlot Slot)
{
	FNCGunSlotData& SlotData = GetSlotData(Slot);
	const FNCGunData* Data = FindGunData(SlotData.GunID);
	if (!Data) return;

	const TObjectPtr<UNCGunComponent>* Found = WeaponComponents.Find(SlotData.GunTypeTag);
	if (!Found || !*Found) return;

	UNCGunComponent* Weapon = *Found;

	Weapon->ActivateGun(Data, SlotData.CurrentAmmo, SlotData.ReserveAmmo);

	Weapon->OnAmmoChanged.RemoveDynamic(this, &UNCEquipmentComponent::OnActiveWeaponAmmoChanged);
	Weapon->OnAmmoChanged.AddDynamic(this, &UNCEquipmentComponent::OnActiveWeaponAmmoChanged);

	Weapon->OnFireModeChanged.RemoveDynamic(this, &UNCEquipmentComponent::OnActiveWeaponFireModeChanged);
	Weapon->OnFireModeChanged.AddDynamic(this, &UNCEquipmentComponent::OnActiveWeaponFireModeChanged);

	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
}

void UNCEquipmentComponent::HideActiveWeaponVisualOnly()
{
	DeactivateCurrentWeapon();
}

void UNCEquipmentComponent::ShowActiveWeaponVisualAgain()
{
	if (ActiveSlot != ENCGunSlot::None)
	{
		ActivateWeaponForSlot(ActiveSlot);
	}
}

void UNCEquipmentComponent::DropOccupantGunForNewGun(FName NewGunID, FVector DropLocation, FRotator DropRotation, float LifeSpanSeconds)
{
	const FName OldGunID = GetOccupantGunID(NewGunID);
	if (OldGunID.IsNone()) return;

	const FNCGunData* OldData = FindGunData(OldGunID);
	if (!OldData || !OldData->GunActorClass) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ANCGunActor* DroppedGun = GetWorld()->SpawnActor<ANCGunActor>(
		OldData->GunActorClass, DropLocation, DropRotation, Params);

	if (DroppedGun)
	{
		DroppedGun->bRespawnEnabled = false;
		const FNCGunSlotData& OldSlot = GetSlotData(OldData->SlotType);
		DroppedGun->SavedCurrentAmmo = OldSlot.CurrentAmmo;

		if (LifeSpanSeconds > 0.f)
		{
			DroppedGun->SetLifeSpan(LifeSpanSeconds);
		}
	}
}

void UNCEquipmentComponent::RefillAllReserveAmmo()
{
	auto RefillSlot = [this](ENCGunSlot Slot, FNCGunSlotData& SlotData)
		{
			if (SlotData.GunID.IsNone()) return;

			const FNCGunData* Data = FindGunData(SlotData.GunID);
			if (!Data) return;

			SlotData.ReserveAmmo = Data->MaxReserveAmmo;

			if (Slot == ActiveSlot)
			{
				if (UNCGunComponent* Weapon = GetActiveWeapon())
				{
					Weapon->ReserveAmmo = Data->MaxReserveAmmo;
					Weapon->OnAmmoChanged.Broadcast(Weapon->CurrentAmmo, Weapon->ReserveAmmo);
				}
			}
		};

	RefillSlot(ENCGunSlot::Shotgun, ShotgunSlot);
	RefillSlot(ENCGunSlot::Rifle, RifleSlot);
	RefillSlot(ENCGunSlot::Sidearm, SidearmSlot);
}

void UNCEquipmentComponent::DeactivateCurrentWeapon()
{
	UNCGunComponent* Weapon = GetActiveWeapon();
	if (!Weapon) return;

	FNCGunSlotData& SlotData = GetSlotData(ActiveSlot);

	SlotData.CurrentAmmo = Weapon->CurrentAmmo;
	SlotData.ReserveAmmo = Weapon->ReserveAmmo;

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
	switch (Slot)
	{
	case ENCGunSlot::Shotgun:
		return ShotgunSlot;
	case ENCGunSlot::Rifle:
		return RifleSlot;
	case ENCGunSlot::Sidearm:
		return SidearmSlot;
	default:
		return SidearmSlot;
	}
}

void UNCEquipmentComponent::OnActiveWeaponAmmoChanged(int32 CurrentAmmo, int32 ReserveAmmo)
{
	// 실시간으로 줄어드는 탄약을 SlotData에도 반영
	if (ActiveSlot != ENCGunSlot::None)
	{
		FNCGunSlotData& SlotData = GetSlotData(ActiveSlot);
		SlotData.CurrentAmmo = CurrentAmmo;
		SlotData.ReserveAmmo = ReserveAmmo;
	}

	OnAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo);
}

void UNCEquipmentComponent::OnActiveWeaponFireModeChanged(ENCFireMode NewFireMode)
{
	OnFireModeChanged.Broadcast(NewFireMode);
}

void UNCEquipmentComponent::AddReserveAmmo(ENCGunSlot Slot, int32 Amount)
{
	if (Slot == ENCGunSlot::None || Amount <= 0) return;

	FNCGunSlotData& SlotData = GetSlotData(Slot);
	SlotData.ReserveAmmo += Amount;   // 총 미보유 시에도 무제한 축적

	if (Slot == ActiveSlot)
	{
		if (UNCGunComponent* Weapon = GetActiveWeapon())
		{
			Weapon->ReserveAmmo = SlotData.ReserveAmmo;
			Weapon->OnAmmoChanged.Broadcast(Weapon->CurrentAmmo, Weapon->ReserveAmmo);
		}
	}

	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
}