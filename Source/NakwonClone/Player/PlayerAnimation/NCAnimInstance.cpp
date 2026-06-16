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

    // 헌호수정 - 파괴 중인 액터 접근 방지
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

    // 찬우수정 - Stop 판정을 위해 현재 Speed를 갱신하기 전에 이전 프레임 Speed 저장
    PreviousSpeed = Speed;

    Speed = Velocity.Size2D();

    Direction = UKismetAnimationLibrary::CalculateDirection(
        Velocity,
        OwnerCharacter->GetActorRotation()
    );

    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();

    bShouldMove = Speed > 3.f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

    // 찬우추가 - 걷기/뛰기/앉기 Stop 애니메이션 전환용 상태 계산
    UpdateStopState();

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

// 찬우추가 - Stop 애니메이션 판정 함수
void UNCAnimInstance::UpdateStopState()
{
    if (!MovementComponent)
    {
        bWantsToStop = false;
        bWasMoving = false;
        StopSpeed = 0.f;
        StopDirection = 0.f;
        return;
    }

    const bool bHasAcceleration = !MovementComponent->GetCurrentAcceleration().IsNearlyZero();

    /*
     * Stop 판정 기준
     *
     * bWasMoving      : 직전까지 이동 중이었는가
     * !bHasAcceleration : 현재 입력이 끊겼는가
     * Speed > 10.f    : 아직 완전히 멈추기 전인가
     * !bIsInAir       : 공중 상태가 아닌가
     *
     * 즉, 이동 입력을 놓았고 캐릭터가 감속 중이면 Stop 애니메이션으로 보낼 수 있음.
     */
    bWantsToStop =
        bWasMoving &&
        !bHasAcceleration &&
        Speed > 10.f &&
        !bIsInAir;

    if (bWantsToStop)
    {
        StopSpeed = PreviousSpeed;
        StopDirection = Direction;
    }

    /*
     * 다음 프레임 Stop 판정에 사용할 이동 여부 저장.
     * 50 이상으로 둔 이유:
     * 아주 작은 미끄러짐/보정 속도 때문에 Stop이 계속 발생하는 걸 방지.
     */
    bWasMoving = Speed > 50.f;
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