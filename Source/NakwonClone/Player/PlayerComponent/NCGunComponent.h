#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Weapon/Gun/GunType/NCGunType.h"

#include "NCGunComponent.generated.h"

class UDataTable;
class UCameraComponent;
class UAnimMontage;
class UNiagaraComponent;

// 무기 컴포넌트 레벨 이벤트 (NCEquipmentComponent에서 재구독)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGunEquipped, FName, GunID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGunUnequipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFireModeChanged, ENCFireMode, NewFireMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwapCompleted, ENCGunSlot, NewSlot);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCGunComponent();

	// ----- 무기 레벨 이벤트 (EquipmentComponent가 구독) -----
	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnFireModeChanged OnFireModeChanged;

	// ----- 현재 탄약 상태 (EquipmentComponent가 Activate 시 설정) -----
	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	int32 CurrentAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	int32 ReserveAmmo = 0;

	// ----- 현재 진행 중인 총기 액션 -----
	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	FGameplayTagContainer ActiveGunActions;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	ENCFireMode CurrentFireMode = ENCFireMode::SemiAuto;

	// ----- 상태 조회 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool HasActiveGun() const { return ActiveGunData != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsReloading() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsADS() const;

	// ----- 활성화 / 비활성화 (EquipmentComponent에서 호출) -----
	void ActivateGun(const FNCGunData* InGunData, int32 InCurrentAmmo, int32 InReserveAmmo);
	void DeactivateGun();

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

	// ----- 발사 모드 전환 -----
	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void ToggleFireMode();

	const FNCGunData* GetActiveGunData() const { return ActiveGunData; }

	// 언이퀍 몽타주 재생 (EquipmentComponent의 SelectSlot에서 호출)
	void PlayUnequipMontage(const FNCGunData* Data);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Gun|ADS")
	float ADSInterpSpeed = 10.f;

private:
	// 현재 활성화된 총기 데이터 (EquipmentComponent가 Activate 시 설정, 원시 포인터)
	const FNCGunData* ActiveGunData = nullptr;

	void FireOnce();
	void OnReloadFinished();

	void ApplyADSFOV();
	void RestoreFOV();
	UCameraComponent* FindCamera() const;

	void PlayGunMontage(const TSoftObjectPtr<UAnimMontage>& MontageSoft);

	UFUNCTION()
	void OnMuzzleFlashFinished(UNiagaraComponent* PSystem);

	void AttachGunMesh(const FNCGunData* Data);
	void DetachGunMesh();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> EquippedGunMeshComp;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> EquippedGunSkelMeshComp;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> MuzzleFlashComp;

	FTimerHandle FullAutoTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle MuzzleFlashTimerHandle;

	float DefaultFOV  = 90.f;
	float TargetFOV   = 90.f;

	// 헌호수정 - 반동 누적 상태
	float CurrentRecoilPitch = 0.f;
	float CurrentRecoilYaw   = 0.f;

	void ApplyRecoil(const FNCGunData* Data);

	FVector DefaultCameraLocation = FVector::ZeroVector;
	FVector TargetCameraLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Camera|ADS")
	FVector ADSCameraLocation = FVector(-10.f, 18.f, 8.f);
};
