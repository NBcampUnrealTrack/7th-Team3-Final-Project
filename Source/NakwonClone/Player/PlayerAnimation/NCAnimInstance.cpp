#include "NCAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
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

    Speed = Velocity.Size2D();
    Direction = UKismetAnimationLibrary::CalculateDirection(
        Velocity,
        OwnerCharacter->GetActorRotation()
    );

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bShouldMove = Speed > 3.f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

    UpdateWeaponAndBlendSpace();

    // 헌호수정 - 전용 서버에서는 IK 계산 불필요
    if (UWorld* World = GetWorld())
    {
        if (!World->IsNetMode(NM_DedicatedServer))
        {
            UpdateLeftHandIK();
        }
    }
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

// 헌호수정 - 양손 IK 위치 업데이트 (클라이언트 전용)
void UNCAnimInstance::UpdateLeftHandIK()
{
    UNCCombatComponent* ActiveCombatComponent = CachedCombatComponent
        ? CachedCombatComponent.Get()
        : CombatComponent.Get();

    // 공격 중에는 IK 끔 (애니메이션 자체에 맡김)
    if (OwnerCharacter)
    {
        UAbilitySystemComponent* ASC = OwnerCharacter->FindComponentByClass<UAbilitySystemComponent>();
        bIsAttacking = ASC ? ASC->HasMatchingGameplayTag(NCWeapon::Action_Attacking) : false;
    }

    if (!ActiveCombatComponent || !ActiveCombatComponent->IsWeaponEquipped() || bIsAttacking)
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

    // 스켈레탈 메시 먼저 시도, 없으면 스태틱 메시 시도
    UMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!WeaponMesh)
        WeaponMesh = WeaponActor->FindComponentByClass<UStaticMeshComponent>();

    if (!WeaponMesh)
    {
        bUseLeftHandIK = false;
        bUseRightHandIK = false;
        return;
    }

    // 왼손 IK
    if (!WeaponData->LeftHandIKSocketName.IsNone())
    {
        LeftHandIKLocation = WeaponMesh->GetSocketLocation(WeaponData->LeftHandIKSocketName);
        bUseLeftHandIK = true;
    }
    else
    {
        bUseLeftHandIK = false;
    }

    // 오른손 IK
    if (!WeaponData->RightHandIKSocketName.IsNone())
    {
        RightHandIKLocation = WeaponMesh->GetSocketLocation(WeaponData->RightHandIKSocketName);
        bUseRightHandIK = true;
    }
    else
    {
        bUseRightHandIK = false;
    }
}