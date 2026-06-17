#include "NCPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Common/NCGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "Item/NCItemActor.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "Player/PlayerComponent/NCInteractionComponent.h"
#include "NakwonClone/Player/PlayerComponent/Locomotion/UNCLocomotionComponent.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"

ANCPlayerCharacter::ANCPlayerCharacter()
{
    InitCamera();
    InitComponents();

    // 헌호수정 - DataTable에서 속도 적용하므로 하드코딩 제거
}

void ANCPlayerCharacter::InitCamera()
{
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ANCPlayerCharacter::InitComponents()
{
    InteractionComponent = CreateDefaultSubobject<UNCInteractionComponent>(TEXT("InteractionComponent"));
    LocomotionComponent = CreateDefaultSubobject<UNCLocomotionComponent>(TEXT("LocomotionComponent"));
    CombatComponent = CreateDefaultSubobject<UNCCombatComponent>(TEXT("CombatComponent"));
}

void ANCPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
        
        // 플레이어 태그 부여
        AbilitySystemComponent->AddLooseGameplayTag(NCCharacter::Player);
        
        if (HasAuthority() && AttackAbilityClass)
        {
            AbilitySystemComponent->GiveAbility(
                FGameplayAbilitySpec(AttackAbilityClass, 1));
        }
    }

    //// TODO: 테스트용 임시 크로우바 장착 - 아이템 픽업 시스템 완성 후 제거
    // if (HasAuthority() && CombatComponent)
    // {
    //     FNCWeaponInstance TestWeapon;
    //     TestWeapon.UniqueID = FGuid::NewGuid();
    //     TestWeapon.WeaponID = FName("Crowbar");
    //     TestWeapon.CurrentDurability = 100.f;
    //     TestWeapon.bIsBroken = false;
    //     CombatComponent->EquipWeapon(TestWeapon);
    // }
}

void ANCPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (APlayerState* NCPS = GetPlayerState())
    {
        PlayerInventoryRef = NCPS->FindComponentByClass<UNCPlayerInventoryComponent>();
        if (PlayerInventoryRef)
        {
            // 헌호수정 - 이중 바인딩 방지 (RemoveDynamic 먼저)
            PlayerInventoryRef->OnItemUsed.RemoveDynamic(this, &ANCPlayerCharacter::OnItemUsed);
            PlayerInventoryRef->OnItemUsed.AddDynamic(this, &ANCPlayerCharacter::OnItemUsed);
        }
    }
}

void ANCPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (APlayerState* NCPS = GetPlayerState())
    {
        PlayerInventoryRef = NCPS->FindComponentByClass<UNCPlayerInventoryComponent>();
        if (PlayerInventoryRef)
        {
            // 헌호수정 - 이중 바인딩 방지 (RemoveDynamic 먼저)
            PlayerInventoryRef->OnItemUsed.RemoveDynamic(this, &ANCPlayerCharacter::OnItemUsed);
            PlayerInventoryRef->OnItemUsed.AddDynamic(this, &ANCPlayerCharacter::OnItemUsed);
        }
    }
}

void ANCPlayerCharacter::Server_SetGait_Implementation(FGameplayTag NewGaitTag)
{
    CurrentGaitTag = NewGaitTag;
    if (LocomotionComponent)
        LocomotionComponent->SetGaitTag(NewGaitTag);
}

void ANCPlayerCharacter::Server_SetStance_Implementation(FGameplayTag NewStanceTag)
{
    CurrentStanceTag = NewStanceTag;

    if (CurrentStanceTag == NCCharacter::Crouch)
    {
        Crouch();
        if (LocomotionComponent)
            LocomotionComponent->SetStanceTag(NewStanceTag);
    }
    else
    {
        UnCrouch();
        if (LocomotionComponent)
        {
            // 헌호수정 - 서버도 StanceTag 먼저 Stand로 변경 후 속도 재적용
            LocomotionComponent->SetStanceTag(NCCharacter::Stand);
            LocomotionComponent->SetGaitTag(CurrentGaitTag);
        }
    }
}

void ANCPlayerCharacter::StartSprint()
{
    // 헌호수정 - 스프린트 잠금 중이면 속도 변경도 막음
    if (LocomotionComponent && LocomotionComponent->IsSprintLocked()) return;

    if (LocomotionComponent)
        LocomotionComponent->StartStaminaDrain();

    CurrentGaitTag = NCCharacter::Sprint;
    if (LocomotionComponent)
        LocomotionComponent->SetGaitTag(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::StopSprint()
{
    //헌호수정
    if (LocomotionComponent)
        LocomotionComponent->StopStaminaDrain();

    CurrentGaitTag = NCCharacter::Jog;
    if (LocomotionComponent)
        LocomotionComponent->SetGaitTag(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::ToggleWalk()
{
    if (CurrentGaitTag == NCCharacter::Walk)
        CurrentGaitTag = NCCharacter::Jog;
    else
        CurrentGaitTag = NCCharacter::Walk;

    if (LocomotionComponent)
        LocomotionComponent->SetGaitTag(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::ToggleCrouch()
{
    // 헌호수정 - 공중에서 앉기 방지
    if (GetCharacterMovement()->IsFalling()) return;

    if (CurrentStanceTag == NCCharacter::Crouch)
    {
        UnCrouch();
        CurrentStanceTag = NCCharacter::Stand;
        if (LocomotionComponent)
        {
            // 헌호수정 - StanceTag 먼저 Stand로 변경 후 속도 재적용
            LocomotionComponent->SetStanceTag(NCCharacter::Stand);
            LocomotionComponent->SetGaitTag(CurrentGaitTag);
        }
    }
    else
    {
        // 헌호수정 - 앉을 때 스프린트 중이면 스태미나 드레인 중지
        if (CurrentGaitTag == NCCharacter::Sprint && LocomotionComponent)
            LocomotionComponent->StopStaminaDrain();

        CurrentGaitTag = NCCharacter::Jog;
        Crouch();
        CurrentStanceTag = NCCharacter::Crouch;
        if (LocomotionComponent)
            LocomotionComponent->SetStanceTag(CurrentStanceTag);
    }
    Server_SetStance(CurrentStanceTag);
}

void ANCPlayerCharacter::OnDead()
{
    UE_LOG(LogTemp, Warning, TEXT("[OnDead] 호출됨!"));
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // 헌호수정 - 사망 시 스태미나 타이머 정리
    if (LocomotionComponent)
        LocomotionComponent->ClearAllStaminaTimers();
}

void ANCPlayerCharacter::OnItemUsed(FGameplayTag UsedItemTag)
{
    if (UseItemMontage)
    {
        PlayAnimMontage(UseItemMontage);
    }
}

void ANCPlayerCharacter::OnUseItemMontageEnded()
{
    if (!PlayerInventoryRef || !PlayerInventoryRef->bHasPendingConsumable)
    {
        return;
    }

    FConsumableItemData& Data = PlayerInventoryRef->PendingConsumableData;
    PlayerInventoryRef->bHasPendingConsumable = false;

    UAbilitySystemComponent* NCASC = GetAbilitySystemComponent();
    if (!NCASC)
    {
        return;
    }
    
    if (Data.HealAmount > 0.f)
    {
        const float Current = NCASC->GetNumericAttribute(UVGPlayerAttributeSet::GetHealthAttribute());
        const float Max = NCASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxHealthAttribute());
        NCASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetHealthAttribute(),
            FMath::Clamp(Current + Data.HealAmount, 0.f, Max));
    }

    if (Data.StaminaAmount > 0.f)
    {
        const float Current = NCASC->GetNumericAttribute(UVGPlayerAttributeSet::GetStaminaAttribute());
        const float Max = NCASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxStaminaAttribute());
        NCASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetStaminaAttribute(),
            FMath::Clamp(Current + Data.StaminaAmount, 0.f, Max));
    }

    if (Data.InfectionReduceAmount > 0.f)
    {
        const float Current = NCASC->GetNumericAttribute(UVGPlayerAttributeSet::GetInfectionAttribute());
        NCASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetInfectionAttribute(),
            FMath::Max(Current - Data.InfectionReduceAmount, 0.f));
    }
}
