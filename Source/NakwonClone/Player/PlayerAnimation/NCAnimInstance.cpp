// Fill out your copyright notice in the Description page of Project Settings.

#include "NCAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NCCombatComponent.h"
#include "KismetAnimationLibrary.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Common/NCGameplayTags.h"

void UNCAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        //H
        CombatComponent = OwnerCharacter->FindComponentByClass<UNCCombatComponent>(); 

        // CombatComponent 캐싱
        if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
        {
            CachedCombatComponent = PlayerCharacter->GetCombatComponent();
        }
    }
}

void UNCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        if (!OwnerCharacter || !MovementComponent) return;
    }

    const FVector Velocity = OwnerCharacter->GetVelocity();

    Speed = Velocity.Size2D();
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bShouldMove = Speed > 3.f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
    //H
    UpdateWeaponAndBlendSpace();
}

void UNCAnimInstance::UpdateWeaponAndBlendSpace()
{
    bHasWeapon = false;
    CurrentWeaponTypeTag = FGameplayTag();

    // CombatComponent가 뒤늦게 붙는 케이스 방어
    if (!CombatComponent && OwnerCharacter)
    {
        CombatComponent = OwnerCharacter->FindComponentByClass<UNCCombatComponent>();
    }

    const FNCWeaponData* WeaponData = nullptr;
    if (CombatComponent && CombatComponent->GetEquippedWeapon().IsValid())
    {
        WeaponData = CombatComponent->GetEquippedWeaponData();
        if (WeaponData)
        {
            bHasWeapon = true;
            CurrentWeaponTypeTag = WeaponData->WeaponTypeTag;
        }
    }

    // 4분기: 무기 있음/없음 × 서기/앉기
    if (WeaponData)
    {
        CurrentLocomotionBlendSpace = bIsCrouching
            ? WeaponData->LocomotionBS_Crouching.LoadSynchronous()
            : WeaponData->LocomotionBS_Standing.LoadSynchronous();
    }
    else
    {
        CurrentLocomotionBlendSpace = bIsCrouching
            ? DefaultUnarmedBS_Crouching.LoadSynchronous()
            : DefaultUnarmedBS_Standing.LoadSynchronous();

    // 무기 타입 업데이트
    if (CachedCombatComponent)
    {
        CurrentWeaponTypeTag = CachedCombatComponent->GetEquippedWeaponTypeTag();
        bHasWeapon = !CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_Unarmed);
        bIsOneHandedWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_OneHanded);
        bIsTwoHandedWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_TwoHanded);
    }
}