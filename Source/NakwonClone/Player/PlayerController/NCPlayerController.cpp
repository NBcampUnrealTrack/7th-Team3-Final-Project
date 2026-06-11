#include "NCPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Common/NCGameplayTags.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCInteractionComponent.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Perception/AISense_Hearing.h"
#include "Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

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
        
        // ---하상빈 추가---
        if (InteractAction)
        {
            EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ANCPlayerController::Interact);
        }
        if (InventoryAction)
        {
            EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &ANCPlayerController::ToggleInventory);
        }
        if (QuickSlot1Action)
        {
            EIC->BindAction(QuickSlot1Action, ETriggerEvent::Started, this, &ANCPlayerController::QuickSlot1);
        }
        if (QuickSlot2Action)
        {
            EIC->BindAction(QuickSlot2Action, ETriggerEvent::Started, this, &ANCPlayerController::QuickSlot2);
        }
        if (QuickSlot3Action)
        {
            EIC->BindAction(QuickSlot3Action, ETriggerEvent::Started, this, &ANCPlayerController::QuickSlot3);
        }
        if (QuickSlot4Action)
        {
            EIC->BindAction(QuickSlot4Action, ETriggerEvent::Started, this, &ANCPlayerController::QuickSlot4);
        }
        if (UnArmAction)
        {
            EIC->BindAction(UnArmAction, ETriggerEvent::Started, this, &ANCPlayerController::UnArm);
        }
        // -----------
        
        if (AttackAction)
            EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ANCPlayerController::Attack);
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
    {
        PC->StartSprint();
    }
    
    if (APawn* MyPawn = GetPawn())
    {
        UE_LOG(LogAIPc, Warning, TEXT("[Sprint] ReportNoiseEvent 호출 위치: %s"), *MyPawn->GetActorLocation().ToString());
        
        UAISense_Hearing::ReportNoiseEvent(
            GetWorld(),
            GetPawn()->GetActorLocation(),
            1.0f,
            MyPawn,
            0.0f);
    }
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

void ANCPlayerController::Attack()
{
    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return;

    UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
    if (!ASC) return;

    // 첫 번째 공격: GA_Attack 활성화 시도
    // 실패(이미 공격 중) → 콤보 다음 섹션으로 전환
    if (!ASC->TryActivateAbilityByClass(PC->AttackAbilityClass))
    {
        if (UNCCombatComponent* Combat = PC->FindComponentByClass<UNCCombatComponent>())
        {
            Combat->MeleeAttack();
        }
    }
}

void ANCPlayerController::ToggleInventory()
{
    bIsInventoryOpen = !bIsInventoryOpen;

    ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetPawn());
    
    if (bIsInventoryOpen)
    {
        bShowMouseCursor = true;
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);
        
        if (PlayerCharacter)
        {
            PlayerCharacter->StateTags.AddTag(NCCharacter::InventoryOpen);
        }
    }
    else
    {
        bShowMouseCursor = false;
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        
        if (PlayerCharacter)
        {
            PlayerCharacter->StateTags.RemoveTag(NCCharacter::InventoryOpen);
        }
    }

    OnInventoryToggled.Broadcast(bIsInventoryOpen);
}

void ANCPlayerController::QuickSlot1()
{
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseQuickSlot(0);
    }
}

void ANCPlayerController::QuickSlot2()
{
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseQuickSlot(1);
    }
}

void ANCPlayerController::QuickSlot3()
{
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseQuickSlot(2);
    }
}

void ANCPlayerController::QuickSlot4()
{
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseQuickSlot(3);
    }
}

void ANCPlayerController::UnArm()
{
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->ForceUnArm();
    }
}
