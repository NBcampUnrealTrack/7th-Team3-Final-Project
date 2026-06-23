#include "NCPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Common/NCGameplayTags.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCInteractionComponent.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Item/ANCLootBoxActor.h"
#include "NakwonClone/UI/Inventroy/LootBox/NCLootBoxHud.h"
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
            EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &ANCPlayerController::OnInventoryKey);
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
        if (CloseUIAction)
        {
            EIC->BindAction(CloseUIAction, ETriggerEvent::Started, this, &ANCPlayerController::HandleCloseUI);
        }
        // -----------
        
        if (AttackAction)
            EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ANCPlayerController::Attack);

        // 헌호수정 - 플래시라이트 T키 바인딩
        if (FlashlightAction)
            EIC->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ANCPlayerController::ToggleFlashlight);
    }
}

void ANCPlayerController::Move(const FInputActionValue& Value)
{
    if (IsMenuBlockingInput())
    {
        return;
    }

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
    if (IsMenuBlockingInput())
    {
        return;
    }
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddYawInput(LookAxisVector.X);
    AddPitchInput(LookAxisVector.Y);
}

void ANCPlayerController::StartSprint()
{
    if (IsMenuBlockingInput())
    {
        return;
    }
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
            1.0f, // 소음 크기 : 최대 범위 x 1.0f = 청각 범위 (UI 필요)
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
    if (IsMenuBlockingInput())
    {
        return;
    }
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
    if (IsAttacking()) return; //헌호수정 - 공격 중 상호작용 차단
    if (IsMenuBlockingInput())
    {
        return;
    }
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
    if (IsMenuBlockingInput())
    {
        return;
    }
    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return;

    // 헌호수정 - 공격 시 카메라 방향으로 캐릭터 즉시 회전
    FRotator ControlRot = GetControlRotation();
    PC->SetActorRotation(FRotator(0.f, ControlRot.Yaw, 0.f));

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

void ANCPlayerController::OnInventoryKey()
{
    if (bIsPauseMenuOpen)
    {
        return;
    }
    ToggleInventory();
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
        if (LootBoxWidget)
        {
            CloseLootBoxUI();
        }

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

void ANCPlayerController::HandleCloseUI()
{
    if (TryCloseTopUI())
    {
        return;
    }
    OnPauseRequested.Broadcast();
}

void ANCPlayerController::SetPauseMenuOpen(bool bOpen)
{
    bIsPauseMenuOpen = bOpen;
}

void ANCPlayerController::CloseLootBoxUI()
{
    if (LootBoxWidget)
    {
        LootBoxWidget->CloseLootBoxUI();
        LootBoxWidget = nullptr;
    }
}

bool ANCPlayerController::TryCloseTopUI()
{
    if (bIsInventoryOpen)
    {
        ToggleInventory();
        return true;
    }
    if (LootBoxWidget)
    {
        CloseLootBoxUI();
        return true;
    }
    return false;
}

void ANCPlayerController::QuickSlot1()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 무기 변경 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->ApplyPreset(0);
    }
}

void ANCPlayerController::QuickSlot2()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 무기 변경 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->ApplyPreset(1);
    }
}

void ANCPlayerController::QuickSlot3()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 아이템 사용 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseConsumableSlot(0);
    }
}

void ANCPlayerController::QuickSlot4()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 아이템 선택 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseConsumableSlot(0);
    }
}

void ANCPlayerController::UnArm()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 무기 해제 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->ForceUnArm();
    }
}

bool ANCPlayerController::IsAttacking() const //헌호수정 - 공격 중 체크
{
    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return false;
    UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
    if (!ASC) return false;
    return ASC->HasMatchingGameplayTag(NCWeapon::Action_Attacking);
}

void ANCPlayerController::ToggleFlashlight() //헌호수정
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        PC->ToggleFlashlight();
}

void ANCPlayerController::Client_OpenLootBoxUI_Implementation(AANCLootBoxActor* TargetBox)
{
    if (!TargetBox || !LootBoxWidgetClass)
    {
        return;
    }
    
    if (!bIsInventoryOpen)
    {
        ToggleInventory();
    }

    LootBoxWidget = CreateWidget<UNCLootBoxHud>(this, LootBoxWidgetClass);
    if (LootBoxWidget)
    {
        LootBoxWidget->InitWithLootBox(TargetBox, nullptr);
        LootBoxWidget->AddToViewport();
    }
}