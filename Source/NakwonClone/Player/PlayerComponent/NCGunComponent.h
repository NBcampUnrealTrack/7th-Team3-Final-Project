#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Weapon/Gun/GunType/NCGunType.h"
#include "TimerManager.h"
#include "NCGunComponent.generated.h"

class UCameraComponent;
class UAnimMontage;
class UNiagaraComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAmmoChanged,
	int32,
	CurrentAmmo,
	int32,
	ReserveAmmo
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnGunEquipped,
	FName,
	GunID
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnGunUnequipped
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnFireModeChanged,
	ENCFireMode,
	NewFireMode
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSwapCompleted,
	ENCGunSlot,
	NewSlot
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGunSlotChanged,
	ENCGunSlot,
	Slot,
	FName,
	GunID
);

UCLASS(
	ClassGroup = (Custom),
	Blueprintable,
	meta = (BlueprintSpawnableComponent)
)
class NAKWONCLONE_API UNCGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCGunComponent();

	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Gun|Events")
	FOnFireModeChanged OnFireModeChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	int32 CurrentAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	int32 ReserveAmmo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	FGameplayTagContainer ActiveGunActions;

	UPROPERTY(BlueprintReadOnly, Category = "Gun|State")
	ENCFireMode CurrentFireMode = ENCFireMode::SemiAuto;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool HasActiveGun() const
	{
		return ActiveGunData != nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsFiring() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsReloading() const;

	UFUNCTION(BlueprintCallable, Category = "Gun|Getter")
	bool IsADS() const;

	const FNCGunData* GetActiveGunData() const
	{
		return ActiveGunData;
	}

	float GetDamageMultiplier() const
	{
		return DamageMultiplier;
	}

	void ActivateGun(
		const FNCGunData* InGunData,
		int32 InCurrentAmmo,
		int32 InReserveAmmo
	);

	void DeactivateGun();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	virtual void Reload();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StartADS();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StopADS();

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void ToggleFireMode();

	float PlayUnequipMontage(const FNCGunData* Data);

	void ActivateInfiniteAmmo(float Duration, float InFireRateMultiplier = 1.f);
	void EndInfiniteAmmo();

	void ActivateDamageBoost(float Multiplier, float Duration);
	void EndDamageBoost();

	UMeshComponent* GetEquippedGunMeshComponent() const
	{
		if (EquippedGunSkelMeshComp) return EquippedGunSkelMeshComp;
		return EquippedGunMeshComp;
	}

	UNiagaraComponent* GetMuzzleFlashComponent() const { return MuzzleFlashComp; }

protected:
	virtual void OnBeforeFire()
	{
	}

	virtual void BeginPlay() override;

	virtual void EndPlay(
		const EEndPlayReason::Type EndPlayReason
	) override;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	bool bFireInputHeld = false;

private:
	const FNCGunData* ActiveGunData = nullptr;

	void FireOnce();
	void OnReloadFinished();

	void HideGunMagazine();
	void ShowGunMagazine();
	void DropMagazineMesh();

	void ApplyADSFOV();
	void RestoreFOV();

	UCameraComponent* FindCamera();

	float PlayGunMontage(
		const TSoftObjectPtr<UAnimMontage>& MontageSoft
	);

	UFUNCTION()
	void OnMuzzleFlashFinished(
		UNiagaraComponent* PSystem
	);

	void AttachGunMesh(const FNCGunData* Data);
	void DetachGunMesh();

	void ApplyRecoil(const FNCGunData* Data);

	UPROPERTY()
	TArray<TObjectPtr<UCameraComponent>> CachedCameras;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> EquippedGunMeshComp = nullptr;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> EquippedGunSkelMeshComp = nullptr;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> MuzzleFlashComp = nullptr;

	bool bWantsADS = false;
	bool bInfiniteAmmoActive = false;
	float FireRateMultiplier = 1.f;

	float LastFireTime = -100.f;

	float DefaultFOV = 90.f;
	float TargetFOV = 90.f;

	float CurrentRecoilPitch = 0.f;
	float CurrentRecoilYaw = 0.f;

	float DamageMultiplier = 1.f;

	FTimerHandle ShowMagazineTimerHandle;
	FTimerHandle FullAutoTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FTimerHandle MuzzleFlashTimerHandle;
	FTimerHandle InfiniteAmmoTimerHandle;
	FTimerHandle DamageBoostTimerHandle;

	FVector DefaultCameraLocation = FVector::ZeroVector;
	FVector TargetCameraLocation = FVector::ZeroVector;

	FRotator DefaultCameraRotation = FRotator::ZeroRotator;
	FRotator TargetCameraRotation = FRotator::ZeroRotator;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS",
		meta = (
			AllowPrivateAccess = "true",
			ClampMin = "0.0",
			UIMin = "0.0",
			UIMax = "30.0"
		)
	)
	float ADSInterpSpeed = 10.f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|Location",
		meta = (
			AllowPrivateAccess = "true",
			MakeEditWidget = "true"
		)
	)
	FVector ADSCameraLocation = FVector(0.f, 50.f, 6.f);

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|Location",
		meta = (
			AllowPrivateAccess = "true"
		)
	)
	bool bUseRelativeADSCameraLocation = false;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|Rotation",
		meta = (
			AllowPrivateAccess = "true"
		)
	)
	FRotator ADSCameraRotation = FRotator::ZeroRotator;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|Rotation",
		meta = (
			AllowPrivateAccess = "true"
		)
	)
	bool bUseRelativeADSCameraRotation = true;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|Pivot",
		meta = (
			AllowPrivateAccess = "true",
			MakeEditWidget = "true"
		)
	)
	FVector ADSCameraPivotOffset = FVector::ZeroVector;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|Pivot",
		meta = (
			AllowPrivateAccess = "true"
		)
	)
	FRotator ADSCameraPivotRotation = FRotator::ZeroRotator;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|FOV",
		meta = (
			AllowPrivateAccess = "true"
		)
	)
	bool bOverrideADSFOV = false;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Camera|ADS|FOV",
		meta = (
			AllowPrivateAccess = "true",
			EditCondition = "bOverrideADSFOV",
			ClampMin = "5.0",
			ClampMax = "170.0",
			UIMin = "20.0",
			UIMax = "120.0"
		)
	)
	float ADSFOV = 70.f;
};