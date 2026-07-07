#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/Gun/GunType/NCGunType.h"
#include "Player/PlayerComponent/NCGunComponent.h"

#include "NCEquipmentComponent.generated.h"

class UDataTable;
class UTexture2D;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCEquipmentComponent();

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnGunEquipped OnGunEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnGunUnequipped OnGunUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnFireModeChanged OnFireModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnSwapCompleted OnSwapCompleted;

	// 주무기/보조무기 슬롯 UI 아이콘 동기화용
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
	FOnGunSlotChanged OnGunSlotChanged;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Data")
	TObjectPtr<UDataTable> GunDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Data")
	float SwapDelay = 0.3f;

	// Equip 몽타주가 먼저 보이고, 실제 무기 활성화는 이 시간 뒤에 처리
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Data")
	float EquipSpawnDelay = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Equipment|Weapons")
	TMap<FGameplayTag, TObjectPtr<UNCGunComponent>> WeaponComponents;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	FNCGunSlotData PrimarySlot;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	FNCGunSlotData SecondarySlot;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	ENCGunSlot ActiveSlot = ENCGunSlot::None;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	bool HasActiveGun() const { return ActiveSlot != ENCGunSlot::None; }

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	bool IsSwapping() const { return bIsSwapping; }

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	bool IsReloading() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	bool IsADS() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	FName GetActiveGunID() const;

	// 핫바 아이콘 표시용
	UFUNCTION(BlueprintCallable, Category = "Equipment|Getter")
	UTexture2D* GetGunIcon(ENCGunSlot Slot) const;

	FName GetOccupantGunID(FName ForGunID) const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	bool EquipGun(FName GunID);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	bool EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	void UnequipGun(ENCGunSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	void SelectSlot(ENCGunSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
	void StartADS();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
	void StopADS();

	UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
	void ToggleFireMode();

	const FNCGunData* GetActiveGunData() const;
	const FNCGunData* GetGunData(FName InGunID) const { return FindGunData(InGunID); }

	UNCGunComponent* GetActiveWeapon() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool bIsSwapping = false;

	FTimerHandle SwapTimerHandle;
	FTimerHandle EquipDelayTimerHandle;

	ENCGunSlot PendingEquipSlot = ENCGunSlot::None;

	void ActivateWeaponForSlot(ENCGunSlot Slot);
	void ActivatePendingWeapon();
	void DeactivateCurrentWeapon();
	void OnSwapFinished(ENCGunSlot TargetSlot);

	const FNCGunData* FindGunData(FName GunID) const;
	FNCGunSlotData& GetSlotData(ENCGunSlot Slot);

	UFUNCTION()
	void OnActiveWeaponAmmoChanged(int32 CurrentAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void OnActiveWeaponFireModeChanged(ENCFireMode NewFireMode);
};