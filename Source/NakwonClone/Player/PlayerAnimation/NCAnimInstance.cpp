#include "NCAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"

void UNCAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    UpdateReferences();
}

void UNCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (OwnerCharacter && OwnerCharacter->IsActorBeingDestroyed())
    {
        OwnerCharacter = nullptr;
        MovementComponent = nullptr;
        CombatComponent = nullptr;
        CachedCombatComponent = nullptr;
        return;
    }

    if (!OwnerCharacter || !MovementComponent)
    {
        UpdateReferences();

        if (!OwnerCharacter || !MovementComponent)
        {
            return;
        }
    }

    UpdateLocomotion();
    UpdateStopState(DeltaSeconds);
    UpdateWeaponAndBlendSpace();
    UpdateWeaponStateTags();

    if (UWorld* World = GetWorld())
    {
        if (!World->IsNetMode(NM_DedicatedServer))
        {
            UpdateLeftHandIK();
        }
    }
}

void UNCAnimInstance::UpdateReferences()
{
    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());

    if (!OwnerCharacter)
    {
        MovementComponent = nullptr;
        CombatComponent = nullptr;
        CachedCombatComponent = nullptr;
        return;
    }

    MovementComponent = OwnerCharacter->GetCharacterMovement();
    CombatComponent = OwnerCharacter->FindComponentByClass<UNCCombatComponent>();

    if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
    {
        CachedCombatComponent = PlayerCharacter->GetCombatComponent();
    }
}

void UNCAnimInstance::UpdateLocomotion()
{
    if (!OwnerCharacter || !MovementComponent)
    {
        return;
    }

    const FVector Velocity = OwnerCharacter->GetVelocity();

    VerticalVelocity = Velocity.Z;
    PreviousSpeed = Speed;
    Speed = Velocity.Size2D();

    Direction = UKismetAnimationLibrary::CalculateDirection(
        Velocity,
        OwnerCharacter->GetActorRotation()
    );

    const FRotator ControlRotation = OwnerCharacter->GetControlRotation();
    const FRotator ActorRotation = OwnerCharacter->GetActorRotation();

    const FRotator AimDeltaRotation =
        UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

    AimYaw = FMath::Clamp(AimDeltaRotation.Yaw, -70.f, 70.f);
    AimPitch = FMath::Clamp(AimDeltaRotation.Pitch, -45.f, 45.f);

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    CrouchMovePlayRate = (bIsCrouching && Speed >= 240.f) ? 1.25f : 1.0f;

    bShouldMove =
        Speed > 3.f &&
        !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
}

void UNCAnimInstance::UpdateStopState(float DeltaSeconds)
{
    if (!MovementComponent)
    {
        bWantsToStop = false;
        bWasMoving = false;
        bPreviousWantsToStop = false;
        bDisableIKDuringStop = false;
        StopIKDisableTimer = 0.f;
        StopSpeed = 0.f;
        StopDirection = 0.f;
        return;
    }

    const bool bHasAcceleration =
        !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

    bWantsToStop =
        bWasMoving &&
        !bHasAcceleration &&
        Speed > 10.f &&
        !bIsInAir;

    const bool bJustStartedStop = bWantsToStop && !bPreviousWantsToStop;

    if (bJustStartedStop)
    {
        StopSpeed = PreviousSpeed;
        StopDirection = Direction;
        StopIKDisableTimer = StopIKDisableDuration;
    }

    if (StopIKDisableTimer > 0.f)
    {
        StopIKDisableTimer -= DeltaSeconds;
        bDisableIKDuringStop = true;
    }
    else
    {
        StopIKDisableTimer = 0.f;
        bDisableIKDuringStop = false;
    }

    bWasMoving = Speed > 50.f;
    bPreviousWantsToStop = bWantsToStop;
}

void UNCAnimInstance::UpdateWeaponAndBlendSpace()
{
    if (!CachedCombatComponent && OwnerCharacter)
    {
        if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
        {
            CachedCombatComponent = PlayerCharacter->GetCombatComponent();
        }
    }

    if (!CombatComponent && OwnerCharacter)
    {
        CombatComponent = OwnerCharacter->FindComponentByClass<UNCCombatComponent>();
    }

    UNCCombatComponent* ActiveCombatComponent =
        CachedCombatComponent ? CachedCombatComponent.Get() : CombatComponent.Get();

    if (ActiveCombatComponent)
    {
        CurrentWeaponTypeTag = ActiveCombatComponent->GetEquippedWeaponTypeTag();

        bIsUnarmed = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_Unarmed);
        bHasWeapon = !bIsUnarmed;

        bIsOneHandedWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_OneHanded);
        bIsTwoHandedWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_TwoHanded);
        bIsPistolWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_Pistol);
        bIsShotgunWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_Shotgun);
        bIsRifleWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_Rifle);
    }
    else
    {
        CurrentWeaponTypeTag = NCWeapon::Type_Unarmed;

        bIsUnarmed = true;
        bHasWeapon = false;
        bIsOneHandedWeapon = false;
        bIsTwoHandedWeapon = false;
        bIsPistolWeapon = false;
        bIsShotgunWeapon = false;
        bIsRifleWeapon = false;
    }

    CurrentLocomotionBlendSpace = nullptr;
}

void UNCAnimInstance::UpdateWeaponStateTags()
{
    UAbilitySystemComponent* ASC = OwnerCharacter
        ? OwnerCharacter->FindComponentByClass<UAbilitySystemComponent>()
        : nullptr;

    if (!ASC)
    {
        bIsAttacking = false;
        bIsAiming = false;
        bIsFiring = false;
        bIsReloading = false;
        bIsEmptyReloading = false;
        bIsSwappingWeapon = false;
        bIsChambering = false;
        bIsPumpAction = false;
        return;
    }

    bIsAttacking = ASC->HasMatchingGameplayTag(NCWeapon::Action_Attacking);
    bIsAiming = ASC->HasMatchingGameplayTag(NCWeapon::Action_Aiming);
    bIsFiring = ASC->HasMatchingGameplayTag(NCWeapon::Action_Firing);
    bIsReloading = ASC->HasMatchingGameplayTag(NCWeapon::Action_Reloading);
    bIsEmptyReloading = ASC->HasMatchingGameplayTag(NCWeapon::Action_EmptyReloading);
    bIsSwappingWeapon = ASC->HasMatchingGameplayTag(NCWeapon::Action_Swapping);
    bIsChambering = ASC->HasMatchingGameplayTag(NCWeapon::Action_Chambering);
    bIsPumpAction = ASC->HasMatchingGameplayTag(NCWeapon::Action_PumpAction);
}

void UNCAnimInstance::UpdateLeftHandIK()
{
    UNCCombatComponent* ActiveCombatComponent =
        CachedCombatComponent ? CachedCombatComponent.Get() : CombatComponent.Get();

    const bool bDisableIKDuringStandingRun =
        bIsTwoHandedWeapon &&
        !bIsCrouching &&
        Speed >= TwoHandIKDisableRunSpeed;

    const bool bDisableIKDuringCrouchRun =
        bIsTwoHandedWeapon &&
        bIsCrouching &&
        Speed >= 240.f;

    const bool bDisableIKDuringCrouchIdle =
        bIsTwoHandedWeapon &&
        bIsCrouching &&
        Speed < 10.f;

    const bool bShouldDisableIK =
        bIsAttacking ||
        bDisableIKDuringStop ||
        bDisableIKDuringStandingRun ||
        bDisableIKDuringCrouchRun ||
        bDisableIKDuringCrouchIdle;

    if (!ActiveCombatComponent ||
        !ActiveCombatComponent->IsWeaponEquipped() ||
        bShouldDisableIK)
    {
        bUseLeftHandIK = false;
        bUseRightHandIK = false;
        return;
    }

    const FNCWeaponData* WeaponData = ActiveCombatComponent->GetEquippedWeaponData();

    if (!WeaponData || !WeaponData->bUseTwoHandIK)
    {
        bUseLeftHandIK = false;
        bUseRightHandIK = false;
        return;
    }

    AActor* WeaponActor = ActiveCombatComponent->GetSpawnedWeaponActor();

    if (!WeaponActor)
    {
        bUseLeftHandIK = false;
        bUseRightHandIK = false;
        return;
    }

    UMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();

    if (!WeaponMesh)
    {
        WeaponMesh = WeaponActor->FindComponentByClass<UStaticMeshComponent>();
    }

    if (!WeaponMesh || !OwnerCharacter || !OwnerCharacter->GetMesh())
    {
        bUseLeftHandIK = false;
        bUseRightHandIK = false;
        return;
    }

    if (!WeaponData->LeftHandIKSocketName.IsNone())
    {
        const FVector SocketWorldLocation =
            WeaponMesh->GetSocketLocation(WeaponData->LeftHandIKSocketName);

        LeftHandIKLocation =
            OwnerCharacter->GetMesh()
            ->GetComponentTransform()
            .InverseTransformPosition(SocketWorldLocation);

        bUseLeftHandIK = true;
    }
    else
    {
        bUseLeftHandIK = false;
    }

    if (!WeaponData->RightHandIKSocketName.IsNone())
    {
        const FVector SocketWorldLocation =
            WeaponMesh->GetSocketLocation(WeaponData->RightHandIKSocketName);

        RightHandIKLocation =
            OwnerCharacter->GetMesh()
            ->GetComponentTransform()
            .InverseTransformPosition(SocketWorldLocation);

        bUseRightHandIK = true;
    }
    else
    {
        bUseRightHandIK = false;
    }
}