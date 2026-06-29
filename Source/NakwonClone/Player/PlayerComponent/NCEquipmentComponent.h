#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapon/Gun/GunType/NCGunType.h"
#include "Player/PlayerComponent/NCGunComponent.h"

#include "NCEquipmentComponent.generated.h"

class UDataTable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCEquipmentComponent();

	// ----- UI 바인딩용 이벤트 -----
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

	// ----- 데이터 -----
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Data")
	TObjectPtr<UDataTable> GunDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Data")
	float SwapDelay = 0.3f;

	// ----- 무기 컴포넌트 맵 (GunTypeTag → 컴포넌트, 캐릭터 BeginPlay에서 등록) -----
	UPROPERTY(BlueprintReadWrite, Category = "Equipment|Weapons")
	TMap<FGameplayTag, TObjectPtr<UNCGunComponent>> WeaponComponents;

	// ----- 슬롯 상태 (탈착 시 탄약 보존용) -----
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	FNCGunSlotData PrimarySlot;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	FNCGunSlotData SecondarySlot;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|State")
	ENCGunSlot ActiveSlot = ENCGunSlot::None;

	// ----- 상태 조회 -----
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

	FName GetOccupantGunID(FName ForGunID) const;

	// ----- 무기 장착 / 해제 -----
	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	bool EquipGun(FName GunID);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	bool EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	void UnequipGun(ENCGunSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment|Manage")
	void SelectSlot(ENCGunSlot Slot);

	// ----- 액션 프록시 (내부적으로 활성 무기에 전달) -----
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

	// ----- 데이터 접근 -----
	const FNCGunData* GetActiveGunData() const;
	const FNCGunData* GetGunData(FName InGunID) const { return FindGunData(InGunID); }

	UNCGunComponent* GetActiveWeapon() const;

protected:
	virtual void BeginPlay() override;

private:
	bool bIsSwapping = false;
	FTimerHandle SwapTimerHandle;

	void ActivateWeaponForSlot(ENCGunSlot Slot);
	void DeactivateCurrentWeapon();
	void OnSwapFinished(ENCGunSlot TargetSlot);
	const FNCGunData* FindGunData(FName GunID) const;
	FNCGunSlotData& GetSlotData(ENCGunSlot Slot);

	UFUNCTION()
	void OnActiveWeaponAmmoChanged(int32 CurrentAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void OnActiveWeaponFireModeChanged(ENCFireMode NewFireMode);
};
