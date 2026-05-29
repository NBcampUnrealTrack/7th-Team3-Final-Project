#include "NCPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGPlayerAttributeSet.h"  // 추가 함
#include "NakwonClone/Common/NCGameplayTags.h"

ANCPlayerCharacter::ANCPlayerCharacter()
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
    
    // --- 추가된 부분 ---
    // AttributeSet 생성 및 ASC에 자동 등록
    PlayerAttributeSet = CreateDefaultSubobject<UVGPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

void ANCPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // --- 추가된 부분 ---
    // AttributeSet 초기화
    if (AbilitySystemComponent)
    {
        // (Owner, Avatar)
        // Owner : ASC를 실제로 소유하는 액터 (데이터 관리 주체)
        // Avatar : ASC가 물리적으로 붙어 있는 액터 (실제 게임 월드에서 활동하는 주체)
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}

void ANCPlayerCharacter::Server_SetGait_Implementation(FGameplayTag NewGaitTag)
{
    CurrentGaitTag = NewGaitTag;
    ApplyMovementData(CurrentGaitTag);
}

void ANCPlayerCharacter::Server_SetStance_Implementation(FGameplayTag NewStanceTag)
{
    CurrentStanceTag = NewStanceTag;

    if (CurrentStanceTag == NCCharacter::Crouch)
    {
        Crouch();
        ApplyMovementData(CurrentStanceTag);
    }
    else
    {
        UnCrouch();
        ApplyMovementData(CurrentGaitTag);
    }
}

void ANCPlayerCharacter::StartSprint()
{
    CurrentGaitTag = NCCharacter::Sprint;
    ApplyMovementData(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::StopSprint()
{
    CurrentGaitTag = NCCharacter::Jog;
    ApplyMovementData(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::ToggleWalk()
{
    if (CurrentGaitTag == NCCharacter::Walk)
    {
        CurrentGaitTag = NCCharacter::Jog;
    }
    else
    {
        CurrentGaitTag = NCCharacter::Walk;
    }
    ApplyMovementData(CurrentGaitTag);
    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::ToggleCrouch()
{
    if (CurrentStanceTag == NCCharacter::Crouch)
    {
        UnCrouch();
        CurrentStanceTag = NCCharacter::Stand;
        ApplyMovementData(CurrentGaitTag);
    }
    else
    {
        Crouch();
        CurrentStanceTag = NCCharacter::Crouch;
        ApplyMovementData(CurrentStanceTag);
    }
    Server_SetStance(CurrentStanceTag);
}