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

    // 찬우추가 - ABP에서 애니메이션 상태값을 읽기 위한 Getter
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

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    bool GetWantsToStop() const { return bWantsToStop; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    bool GetWasMoving() const { return bWasMoving; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    float GetStopSpeed() const { return StopSpeed; }

    UFUNCTION(BlueprintPure, Category = "Locomotion|Stop")
    float GetStopDirection() const { return StopDirection; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    FGameplayTag GetCurrentWeaponTypeTag() const { return CurrentWeaponTypeTag; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetHasWeapon() const { return bHasWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsOneHandedWeapon() const { return bIsOneHandedWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsTwoHandedWeapon() const { return bIsTwoHandedWeapon; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    bool GetIsAttacking() const { return bIsAttacking; }

    UFUNCTION(BlueprintPure, Category = "Locomotion")
    UBlendSpace* GetCurrentLocomotionBlendSpace() const { return CurrentLocomotionBlendSpace; }

    // 찬우추가 - 필요 시 BP/외부에서 값 보정 가능하도록 Setter 제공
    UFUNCTION(BlueprintCallable, Category = "Locomotion|Stop")
    void SetWantsToStop(bool bNewWantsToStop) { bWantsToStop = bNewWantsToStop; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion|Stop")
    void SetStopSpeed(float NewStopSpeed) { StopSpeed = NewStopSpeed; }

    UFUNCTION(BlueprintCallable, Category = "Locomotion|Stop")
    void SetStopDirection(float NewStopDirection) { StopDirection = NewStopDirection; }

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
    bool bIsOneHandedWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsTwoHandedWeapon = false;

    // 찬우추가 - Stop 애니메이션 판정용 변수
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    bool bWantsToStop = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    bool bWasMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    float StopSpeed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Stop", meta = (AllowPrivateAccess = "true"))
    float StopDirection = 0.f;

    // 찬우추가 - 이전 프레임 속도 저장용. ABP에서 직접 쓸 필요는 없어서 UPROPERTY 제외
    float PreviousSpeed = 0.f;

    // 헌호수정 - 양손 IK
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftHandIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector RightHandIKLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    bool bUseLeftHandIK = false;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    bool bUseRightHandIK = false;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

protected:
    void UpdateWeaponAndBlendSpace();
    void UpdateLeftHandIK();

    // 찬우추가 - 걷기/뛰기 Stop 판정 계산 함수
    void UpdateStopState();
};