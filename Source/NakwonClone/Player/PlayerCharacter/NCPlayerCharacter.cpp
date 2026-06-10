#include "NCPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Common/NCGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "Player/PlayerComponent/NCInteractionComponent.h"
#include "NakwonClone/Player/PlayerComponent/Locomotion/UNCLocomotionComponent.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"

ANCPlayerCharacter::ANCPlayerCharacter()
{
    InitCamera();
    InitComponents();

    //H
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
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
    }
}

void ANCPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    if (APlayerState* NCPS = GetPlayerState())
    {
        PlayerInventoryRef = NCPS->FindComponentByClass<UNCPlayerInventoryComponent>();
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
            LocomotionComponent->SetGaitTag(CurrentGaitTag);
    }
}

void ANCPlayerCharacter::StartSprint()
{
    CurrentGaitTag = NCCharacter::Sprint;
    if (LocomotionComponent)
        LocomotionComponent->SetGaitTag(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::StopSprint()
{
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
    if (CurrentStanceTag == NCCharacter::Crouch)
    {
        UnCrouch();
        CurrentStanceTag = NCCharacter::Stand;
        if (LocomotionComponent)
            LocomotionComponent->SetGaitTag(CurrentGaitTag);
    }
    else
    {
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
}
