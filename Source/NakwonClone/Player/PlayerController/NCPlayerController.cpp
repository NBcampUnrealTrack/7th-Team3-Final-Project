#include "NCPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Common/NCGameplayTags.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/Assassination/NCAssassinationComponent.h"
#include "NakwonClone/Player/PlayerComponent/NCInteractionComponent.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h" // 하상빈 추가
#include "Player/PlayerData/NCWeaponData.h" // 하상빈 추가
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

    // 총기 슬롯 전환 완료 시점에 근접무기 장착/해제 연동
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
    {
        if (UNCEquipmentComponent* EquipComp = PC->GetEquipmentComponent())
        {
            EquipComp->OnSwapCompleted.AddDynamic(this, &ANCPlayerController::OnGunSwapCompleted);
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
        if (QuickSlot4Action)
        {
            EIC->BindAction(QuickSlot4Action, ETriggerEvent::Started, this, &ANCPlayerController::QuickSlot4);
        }
        if (QuickSlot5Action)
        {
            EIC->BindAction(QuickSlot5Action, ETriggerEvent::Started, this, &ANCPlayerController::QuickSlot5);
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

        // 하상빈 추가 - 총기 입력 바인딩
        if (GunFireAction)
        {
            EIC->BindAction(GunFireAction, ETriggerEvent::Started,   this, &ANCPlayerController::GunStartFire);
            EIC->BindAction(GunFireAction, ETriggerEvent::Completed, this, &ANCPlayerController::GunStopFire);
        }
        if (GunADSAction)
        {
            EIC->BindAction(GunADSAction, ETriggerEvent::Started,   this, &ANCPlayerController::GunStartADS);
            EIC->BindAction(GunADSAction, ETriggerEvent::Completed, this, &ANCPlayerController::GunStopADS);
        }
        if (GunReloadAction)
            EIC->BindAction(GunReloadAction,         ETriggerEvent::Started, this, &ANCPlayerController::GunReload);
        if (GunToggleFireModeAction)
            EIC->BindAction(GunToggleFireModeAction, ETriggerEvent::Started, this, &ANCPlayerController::GunToggleFireMode);
        if (GunSlot1Action)
            EIC->BindAction(GunSlot1Action, ETriggerEvent::Started, this, &ANCPlayerController::GunSelectRifle);

        if (GunSlot2Action)
            EIC->BindAction(GunSlot2Action, ETriggerEvent::Started, this, &ANCPlayerController::GunSelectShotgun);

        if (GunSlot3Action)
            EIC->BindAction(GunSlot3Action, ETriggerEvent::Started, this, &ANCPlayerController::GunSelectSidearm);
        // 헌호수정 - 암살 Q키 바인딩
        if (AssassinateAction)
            EIC->BindAction(AssassinateAction, ETriggerEvent::Started, this, &ANCPlayerController::Assassinate);
        if (ViewChangeAction)
        {
            EIC->BindAction(
                ViewChangeAction,
                ETriggerEvent::Started,
                this,
                &ANCPlayerController::ToggleView);
        }
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

    //헌호수정 - 정조준(ADS) 중이면 마우스 감도 절반으로 (정밀 조준)
    float SensMultiplier = 1.f;
    if (UNCEquipmentComponent* EC = GetGunComp())
    {
        if (EC->IsADS())
        {
            SensMultiplier = ADSLookSensitivity;
        }
    }

    AddYawInput(LookAxisVector.X * SensMultiplier);
    AddPitchInput(LookAxisVector.Y * SensMultiplier);
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
    if (IsUsingItem()) return; // 소모품 사용 중 상호작용(줍기) 차단

    if (LootBoxWidget)
    {
        LootBoxWidget->CloseLootBoxUI();
        return;
    }

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

    if (IsUsingItem())
    {
        return;
    }

    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC)
    {
        return;
    }

    if (UNCEquipmentComponent* EquipComp = PC->GetEquipmentComponent())
    {
        if (EquipComp->HasActiveGun())
        {
            return;
        }
    }

    //헌호수정 - 근접 공격을 크로스헤어(카메라 조준) 방향으로 나가게 캐릭터 회전
    const FRotator ControlRot = GetControlRotation();
    PC->SetActorRotation(FRotator(0.f, ControlRot.Yaw, 0.f));

    UNCCombatComponent* Combat = PC->FindComponentByClass<UNCCombatComponent>();
    if (!Combat)
    {
        return;
    }

    // 공격 몽타주가 이미 재생 중이면
    // GAS를 다시 켜지 말고 콤보 입력 예약만 한다.
    if (Combat->IsMeleeAttackMontagePlaying())
    {
        Combat->MeleeAttack();
        return;
    }

    UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    const bool bActivated = ASC->TryActivateAbilityByClass(PC->AttackAbilityClass);

    if (bActivated)
    {
        return;
    }

    // 예외 처리:
    // 태그는 남아 있는데 몽타주 상태 확인이 꼬인 경우
    if (ASC->HasMatchingGameplayTag(NCWeapon::Action_Attacking))
    {
        Combat->MeleeAttack();
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
    if (IsUsingItem()) return; // 소모품 사용 중 무기 변경 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->ApplyPreset(0);
    }
}

void ANCPlayerController::QuickSlot2()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 무기 변경 차단
    if (IsUsingItem()) return; // 소모품 사용 중 무기 변경 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->ApplyPreset(1);
    }
}

void ANCPlayerController::QuickSlot4()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 아이템 사용 차단
    if (IsUsingItem()) return; // 이미 사용 중이면 중복 사용 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseConsumableSlot(0);
    }
}

void ANCPlayerController::QuickSlot5()
{
    if (IsAttacking()) return; //헌호수정 - 공격 중 아이템 선택 차단
    if (IsUsingItem()) return; // 이미 사용 중이면 중복 사용 차단
    if (UNCPlayerInventoryComponent* NCInventoryComp = GetPlayerState<APlayerState>()->FindComponentByClass<UNCPlayerInventoryComponent>())
    {
        NCInventoryComp->UseConsumableSlot(1);
    }
}

void ANCPlayerController::UnArm()
{
    if (IsMenuBlockingInput()) return;
    if (IsAttacking()) return;
    if (IsUsingItem()) return;

    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return;

    UNCEquipmentComponent* EquipComp = PC->GetEquipmentComponent();
    if (!EquipComp) return;

    if (EquipComp->IsSwapping()) return;

    if (EquipComp->HasActiveGun())
    {
        EquipComp->SelectSlot(ENCGunSlot::None);
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

bool ANCPlayerController::IsUsingItem() const
{
    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return false;
    UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
    if (!ASC) return false;
    return ASC->HasMatchingGameplayTag(NCWeapon::Action_UsingItem);
}

void ANCPlayerController::ToggleFlashlight() //헌호수정
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        PC->ToggleFlashlight();
}

// ─────────────────────────────────────────────
// 하상빈 추가 - 총기 입력

UNCEquipmentComponent* ANCPlayerController::GetGunComp() const
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
        return PC->GetEquipmentComponent();
    return nullptr;
}

void ANCPlayerController::GunStartFire()
{
    if (IsMenuBlockingInput()) return;
    if (IsUsingItem()) return; // 소모품 사용 중 발사 차단
    if (UNCEquipmentComponent* EC = GetGunComp()) EC->StartFire();
}

void ANCPlayerController::GunStopFire()
{
    if (UNCEquipmentComponent* EC = GetGunComp()) EC->StopFire();
}

void ANCPlayerController::GunStartADS()
{
    if (IsMenuBlockingInput()) return;
    if (IsUsingItem()) return; // 소모품 사용 중 조준 차단

    UNCEquipmentComponent* EC = GetGunComp();
    if (!EC || !EC->HasActiveGun()) return;

    EC->StartADS();
}

void ANCPlayerController::GunStopADS()
{
    UNCEquipmentComponent* EC = GetGunComp();
    if (!EC || !EC->HasActiveGun()) return;

    EC->StopADS();
}

void ANCPlayerController::GunReload()
{
    if (IsMenuBlockingInput()) return;
    if (IsUsingItem()) return; // 소모품 사용 중 재장전 차단
    if (UNCEquipmentComponent* EC = GetGunComp()) EC->Reload();
}

void ANCPlayerController::GunToggleFireMode()
{
    if (IsMenuBlockingInput()) return;
    if (IsUsingItem()) return; // 소모품 사용 중 발사모드 전환 차단
    if (UNCEquipmentComponent* EC = GetGunComp()) EC->ToggleFireMode();
}

bool ANCPlayerController::IsWeaponSwapBusy() const
{
    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return false;

    UNCInteractionComponent* InteractionComp = PC->FindComponentByClass<UNCInteractionComponent>();

    return InteractionComp && InteractionComp->IsPickingUp();
}

void ANCPlayerController::GunSelectShotgun()
{
    GunSelectSlot(ENCGunSlot::Shotgun);
}

void ANCPlayerController::GunSelectRifle()
{
    GunSelectSlot(ENCGunSlot::Rifle);
}

void ANCPlayerController::GunSelectSidearm()
{
    GunSelectSlot(ENCGunSlot::Sidearm);
}

void ANCPlayerController::GunSelectSlot(ENCGunSlot Slot)
{
    if (IsMenuBlockingInput()) return;
    if (IsAttacking()) return;
    if (IsUsingItem()) return;

    ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn());
    if (!PC) return;

    if (UNCAssassinationComponent* AC = PC->FindComponentByClass<UNCAssassinationComponent>())
    {
        if (AC->bIsAssassinating) return;
    }

    UNCEquipmentComponent* EC = PC->GetEquipmentComponent();
    if (!EC) return;

    if (IsWeaponSwapBusy()) return;

    const FNCGunSlotData& SlotData =
        (Slot == ENCGunSlot::Shotgun) ? EC->ShotgunSlot :
        (Slot == ENCGunSlot::Rifle) ? EC->RifleSlot :
        EC->SidearmSlot;

    if (SlotData.GunID.IsNone())
    {
        return;
    }

    EC->SelectSlot(Slot);
}

void ANCPlayerController::Assassinate() //헌호수정 - 암살
{
    if (IsMenuBlockingInput()) return;
    if (IsAttacking()) return;
    if (IsUsingItem()) return; // 소모품 사용 중 암살 차단
    if (UNCEquipmentComponent* EC = GetGunComp()) // 헌호수정 - 총 장착 중 암살 차단
        if (EC->HasActiveGun()) return;
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
    {
        //헌호수정 - 근접무기 장착 중일 때만 암살 가능 (맨손 암살 차단)
        UNCCombatComponent* Combat = PC->FindComponentByClass<UNCCombatComponent>();
        if (!Combat || !Combat->IsWeaponEquipped()) return;

        if (UNCAssassinationComponent* AC = PC->FindComponentByClass<UNCAssassinationComponent>())
            if (AC->bIsAssassinating) return; // 헌호수정 - 암살 중 재입력 차단
        PC->TryAssassinate();
    }
}

void ANCPlayerController::OnGunSwapCompleted(ENCGunSlot NewSlot)
{
    bUnArmPending = false;

    if (ANCPlayerCharacter* PC =
        Cast<ANCPlayerCharacter>(GetPawn()))
    {
        PC->ApplyFirstPersonWeaponCameraOffset(NewSlot);
    }
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

void ANCPlayerController::ToggleView()
{
    if (ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetPawn()))
    {
        PC->ToggleView();
    }
}

void ANCPlayerController::Client_ShowNotification_Implementation(const FText& Message, FLinearColor Color)
{
    OnShowNotification(Message, Color);
}