#include "NCPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
}

void ANCPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
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