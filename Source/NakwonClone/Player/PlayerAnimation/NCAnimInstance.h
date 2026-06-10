// Fill out your copyright notice in the Description page of Project Settings.

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
class UNCCombatComponent;

UCLASS()
class NAKWONCLONE_API UNCAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;
    //H
    UPROPERTY(BlueprintReadOnly, Category = "References")
    TObjectPtr<UNCCombatComponent> CombatComponent;

    // 캐싱 (매 프레임 Find 방지)
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

    //H 현재 활성 이동 BlendSpace (런타임 계산 결과)
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    TObjectPtr<UBlendSpace> CurrentLocomotionBlendSpace;

    //H 무기 없을 때 기본 BlendSpace (ABP 디테일에서 할당)
    UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Defaults")
    TSoftObjectPtr<UBlendSpace> DefaultUnarmedBS_Standing;
    //H
    UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Defaults")
    TSoftObjectPtr<UBlendSpace> DefaultUnarmedBS_Crouching;

protected:
    void UpdateWeaponAndBlendSpace();
    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentWeaponTypeTag;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bHasWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsOneHandedWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    bool bIsTwoHandedWeapon = false;
};