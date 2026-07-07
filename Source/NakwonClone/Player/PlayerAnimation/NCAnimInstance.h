#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "NCAnimInstance.generated.h"

class ACharacter;
class UBlendSpace;
class UNCCombatComponent;
class UCharacterMovementComponent;
class ANCPlayerCharacter;

UCLASS()
class NAKWONCLONE_API UNCAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    float GetSpeed() const { return Speed; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    float GetDirection() const { return Direction; }

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float VerticalVelocity = 0.f;

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    bool GetShouldMove() const { return bShouldMove; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    bool GetIsInAir() const { return bIsInAir; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    bool GetIsCrouching() const { return bIsCrouching; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Crouch")
    float GetCrouchMovePlayRate() const { return CrouchMovePlayRate; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    bool GetWantsToStop() const { return bWantsToStop; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    bool GetWasMoving() const { return bWasMoving; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    float GetStopSpeed() const { return StopSpeed; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    float GetStopDirection() const { return StopDirection; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    bool GetDisableIKDuringStop() const { return bDisableIKDuringStop; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    FGameplayTag GetCurrentWeaponTypeTag() const { return CurrentWeaponTypeTag; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetHasWeapon() const { return bHasWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsUnarmed() const { return bIsUnarmed; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsOneHandedWeapon() const { return bIsOneHandedWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsTwoHandedWeapon() const { return bIsTwoHandedWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsPistolWeapon() const { return bIsPistolWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsShotgunWeapon() const { return bIsShotgunWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsRifleWeapon() const { return bIsRifleWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsAttacking() const { return bIsAttacking; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsAiming() const { return bIsAiming; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsFiring() const { return bIsFiring; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsReloading() const { return bIsReloading; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsEmptyReloading() const { return bIsEmptyReloading; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsSwappingWeapon() const { return bIsSwappingWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsChambering() const { return bIsChambering; }

    UFUNCTION(BlueprintPure, Category = "Weapon|State")
    bool GetIsPumpAction() const { return bIsPumpAction; }

    UFUNCTION(BlueprintPure, Category = "Aim")
    float GetAimYaw() const { return AimYaw; }

    UFUNCTION(BlueprintPure, Category = "Aim")
    float GetAimPitch() const { return AimPitch; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    UBlendSpace* GetCurrentLocomotionBlendSpace() const { return CurrentLocomotionBlendSpace; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion|Stop")
    void SetWantsToStop(bool bNewWantsToStop) { bWantsToStop = bNewWantsToStop; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion|Stop")
    void SetStopSpeed(float NewStopSpeed) { StopSpeed = NewStopSpeed; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion|Stop")
    void SetStopDirection(float NewStopDirection) { StopDirection = NewStopDirection; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsAiming(bool bNewValue) { bIsAiming = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsFiring(bool bNewValue) { bIsFiring = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsReloading(bool bNewValue) { bIsReloading = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsEmptyReloading(bool bNewValue) { bIsEmptyReloading = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsSwappingWeapon(bool bNewValue) { bIsSwappingWeapon = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsChambering(bool bNewValue) { bIsChambering = bNewValue; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|State")
    void SetIsPumpAction(bool bNewValue) { bIsPumpAction = bNewValue; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<UNCCombatComponent> CombatComponent;

    UPROPERTY()
    TObjectPtr<UNCCombatComponent> CachedCombatComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bShouldMove = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Crouch", meta = (AllowPrivateAccess = "true"))
    float CrouchMovePlayRate = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    TObjectPtr<UBlendSpace> CurrentLocomotionBlendSpace;

    UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Defaults")
    TSoftObjectPtr<UBlendSpace> DefaultUnarmedBS_Standing;

    UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Defaults")
    TSoftObjectPtr<UBlendSpace> DefaultUnarmedBS_Crouching;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTypeTag;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bHasWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsUnarmed = true;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsOneHandedWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsTwoHandedWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsPistolWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsShotgunWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsRifleWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsFiring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsReloading = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsEmptyReloading = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsSwappingWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsChambering = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    bool bIsPumpAction = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    bool bWantsToStop = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    bool bWasMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    float StopSpeed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    float StopDirection = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    bool bDisableIKDuringStop = false;

    UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    float StopIKDisableDuration = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float TwoHandIKDisableRunSpeed = 350.f;

    float StopIKDisableTimer = 0.f;
    float PreviousSpeed = 0.f;
    bool bPreviousWantsToStop = false;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftHandIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector RightHandIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    bool bUseLeftHandIK = false;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    bool bUseRightHandIK = false;

    UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
    float AimYaw = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = "true"))
    float AimPitch = 0.f;

protected:
    void UpdateReferences();
    void UpdateLocomotion();
    void UpdateWeaponAndBlendSpace();
    void UpdateWeaponStateTags();
    void UpdateWeaponIK();
    void UpdateStopState(float DeltaSeconds);
};