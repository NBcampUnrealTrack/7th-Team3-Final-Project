#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Weapon/Gun/GunType/NCGunType.h"

#include "NCGunComponent.generated.h"

class UDataTable;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunEquipped, FName, GunID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunUnequipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireModeChanged, ENCFireMode, NewFireMode);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCGunComponent();

	// ----- UI 바인딩용 이벤트 -----
	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnGunEquipped OnGunEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnGunUnequipped OnGunUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnFireModeChanged OnFireModeChanged;

	// ----- 데이터 -----
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Data")
	TObjectPtr<UDataTable> GunDataTable;

	// -----무기 교체 딜레이(애니메이션 길이에 맞춰 BP에서 조정) -----
	UPROPERTY(EditDefaultsOnly, Category = "Gun|Data")
	float SwapDelay = 0.3f;

	// ----- 슬롯 상태 -----
	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	FNCGunSlotData PrimarySlot;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	FNCGunSlotData SecondarySlot;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	ENCGunSlot ActiveSlot = ENCGunSlot::None;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	ENCFireMode CurrentFireMode = ENCFireMode::SemiAuto;

	// ----- 현재 진행 중인 총기 액션 -----
	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	FGameplayTagContainer ActiveGunActions;

	// ----- 상태 조회 헬퍼 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool HasActiveGun() const { return ActiveSlot != ENCGunSlot::None; }

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsReloading() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsADS() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsSwapping() const;

	// ----- 탄약 / 총기 정보 getter -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	FName GetActiveGunID() const;

	//  ----- GunID와 같은 슬롯에 현재 장착된 GunID 반환 -----
	FName GetOccupantGunID(FName ForGunID) const;

	// ----- 무기 장착 / 해제 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Equip")
	bool EquipGun(FName GunID);

	// ----- 남은 탄약 수 아이템에 저장 (인벤토리 연동용) -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Equip")
	bool EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintCallable, Category = "Gun|Equip")
	void UnequipGun(ENCGunSlot Slot);

	// ----- 슬롯 선택 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Equip")
	void SelectSlot(ENCGunSlot Slot);

	// ----- 사격 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StopFire();

	// ----- 재장전 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void Reload();

	// ----- ADS -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StartADS();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StopADS();

	// ----- 발사 모드 전환 (단발 <-> 연사) -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void ToggleFireMode();

	const FNCGunData* GetActiveGunData() const;
	const FNCGunData* GetGunData(FName InGunID) const { return FindGunData(InGunID); }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// FOV 보간 속도 (BP에서 조정 가능)
	UPROPERTY(EditDefaultsOnly, Category = "Gun|ADS")
	float ADSInterpSpeed = 10.f;

private:
	void FireOnce();
	void OnReloadFinished();
	void OnSwapFinished(ENCGunSlot TargetSlot);

	void ApplyADSFOV();
	void RestoreFOV();
	UCameraComponent* FindCamera() const;

	// TODO: 찬우님이 스켈레톤에 총기 전용 소켓 추가하면 DT_GunData HandSocketName에 입력
	void AttachGunMesh(const FNCGunData* Data);
	void DetachGunMesh();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> EquippedGunMeshComp;

	FNCGunSlotData& GetActiveSlotData();
	const FNCGunData* FindGunData(FName GunID) const;

	FTimerHandle FullAutoTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle SwapTimerHandle;

	float DefaultFOV  = 90.f;
	float TargetFOV   = 90.f;
};
