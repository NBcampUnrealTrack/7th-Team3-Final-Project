#include "NCAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemComponent.h"

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

        CombatComponent = OwnerCharacter->FindComponentByClass<UNCCombatComponent>();

        if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
        {
            CachedCombatComponent = PlayerCharacter->GetCombatComponent();
        }
    }
}

void UNCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (OwnerCharacter && OwnerCharacter->IsActorBeingDestroyed())
    {
        OwnerCharacter = nullptr;
        MovementComponent = nullptr;
        return;
    }

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());

        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();

            CombatComponent = OwnerCharacter->FindComponentByClass<UNCCombatComponent>();

            if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
            {
                CachedCombatComponent = PlayerCharacter->GetCombatComponent();
            }
        }

        if (!OwnerCharacter || !MovementComponent)
        {
            return;
        }
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

    if (bIsCrouching && Speed >= 240.f)
    {
        CrouchMovePlayRate = 1.25f;
    }
    else
    {
        CrouchMovePlayRate = 1.0f;
    }

    bShouldMove = Speed > 3.f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

    UpdateStopState(DeltaSeconds);

    UpdateWeaponAndBlendSpace();

    if (UWorld* World = GetWorld())
    {
        if (!World->IsNetMode(NM_DedicatedServer))
        {
            UpdateLeftHandIK();
        }
    }
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

    const bool bHasAcceleration = !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

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

    UNCCombatComponent* ActiveCombatComponent = CachedCombatComponent
        ? CachedCombatComponent.Get()
        : CombatComponent.Get();

    const FNCWeaponData* WeaponData = nullptr;

    if (ActiveCombatComponent)
    {
        CurrentWeaponTypeTag = ActiveCombatComponent->GetEquippedWeaponTypeTag();

        bHasWeapon = !CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_Unarmed);
        bIsOneHandedWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_OneHanded);
        bIsTwoHandedWeapon = CurrentWeaponTypeTag.MatchesTagExact(NCWeapon::Type_TwoHanded);

        WeaponData = ActiveCombatComponent->GetEquippedWeaponData();
    }
    else
    {
        CurrentWeaponTypeTag = NCWeapon::Type_Unarmed;
        bHasWeapon = false;
        bIsOneHandedWeapon = false;
        bIsTwoHandedWeapon = false;
    }

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
    }
}

void UNCAnimInstance::UpdateLeftHandIK()
{
    UNCCombatComponent* ActiveCombatComponent = CachedCombatComponent
        ? CachedCombatComponent.Get()
        : CombatComponent.Get();

    if (OwnerCharacter)
    {
        UAbilitySystemComponent* ASC = OwnerCharacter->FindComponentByClass<UAbilitySystemComponent>();
        bIsAttacking = ASC ? ASC->HasMatchingGameplayTag(NCWeapon::Action_Attacking) : false;
    }

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

    if (!WeaponMesh || !OwnerCharacter->GetMesh())
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

        bUseRightHandIK = false;
    }
    else
    {
        bUseRightHandIK = false;
    }
}