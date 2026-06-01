#include "NCPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCInteractionComponent.h"

ANCPlayerController::ANCPlayerController()
{
}

void ANCPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ANCPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction)
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANCPlayerController::Move);

        if (LookAction)
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANCPlayerController::Look);

        if (SprintAction)
        {
            EIC->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ANCPlayerController::StartSprint);
            EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANCPlayerController::StopSprint);
        }

        if (WalkAction)
            EIC->BindAction(WalkAction, ETriggerEvent::Started, this, &ANCPlayerController::ToggleWalk);

        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ANCPlayerController::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ANCPlayerController::StopJump);
        }

        if (CrouchAction)
            EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &ANCPlayerController::ToggleCrouch);
        
        if (InteractAction)
        {
            EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ANCPlayerController::Interact);
        }
    }
}

void ANCPlayerController::Move(const FInputActionValue& Value)
{
    ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PlayerCharacter) return;

    FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator Rotation = GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    PlayerCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
    PlayerCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void ANCPlayerController::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddYawInput(LookAxisVector.X);
    AddPitchInput(LookAxisVector.Y);
}

void ANCPlayerController::StartSprint()
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        PC->StartSprint();
}

void ANCPlayerController::StopSprint()
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        PC->StopSprint();
}

void ANCPlayerController::ToggleWalk()
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        PC->ToggleWalk();
}

void ANCPlayerController::Jump()
{
    if (ACharacter* Char = Cast<ACharacter>(GetPawn()))
        Char->Jump();
}

void ANCPlayerController::StopJump()
{
    if (ACharacter* Char = Cast<ACharacter>(GetPawn()))
        Char->StopJumping();
}

void ANCPlayerController::ToggleCrouch()
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        PC->ToggleCrouch();
}

void ANCPlayerController::Interact()
{
    if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetPawn()))
    {
        if (UNCInteractionComponent* InteractionComp = PlayerCharacter->FindComponentByClass<UNCInteractionComponent>())
        {
            InteractionComp->Interact();
        }
    }
}
