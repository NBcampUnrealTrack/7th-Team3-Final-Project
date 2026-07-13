#include "NCPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Common/NCGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "Item/NCItemActor.h"
#include "NakwonClone/Framwork/GameInstacne/NCGameInstance.h"
#include "NakwonClone/Player/Assassination/NCAssassinationComponent.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerComponent/Locomotion/UNCLocomotionComponent.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "NakwonClone/Zombie/AI/AttackSlot/VGAttackSlotComponent.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"
#include "Player/PlayerComponent/NCGunComponent.h"
#include "Player/PlayerComponent/NCInteractionComponent.h"
#include "Player/PlayerComponent/NCPistolComponent.h"
#include "Player/PlayerComponent/NCRifleComponent.h"
#include "Player/PlayerComponent/NCShotgunComponent.h"
#include "Player/PlayerData/NCWeaponData.h"
#include "UI/InGame/NCADSHUD.h"
#include "UI/InGame/NCBackpackHUD.h"
#include "UI/InGame/NCHPBar.h"

ANCPlayerCharacter::ANCPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    InitCamera();
    InitComponents();

    // 헌호수정 - DataTable에서 속도 적용하므로 하드코딩 제거
}

UNCGunComponent* ANCPlayerCharacter::GetGunComponent() const
{
	return EquipmentComponent ? EquipmentComponent->GetActiveWeapon() : nullptr;
}

UTexture2D* ANCPlayerCharacter::GetMeleeIcon() const
{
	if (StoredMeleeWeaponID.IsNone()) return nullptr;

	UNCGameInstance* GI = Cast<UNCGameInstance>(GetGameInstance());
	if (!GI) return nullptr;

	FNCWeaponData* Data = GI->GetWeaponData(StoredMeleeWeaponID);
	return Data ? Data->Icon : nullptr;
}

void ANCPlayerCharacter::InitCamera()
{
    // ─────────────────────────────────────────────
    // 3인칭 카메라

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(
        TEXT("CameraBoom")
    );

    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->ProbeSize = 5.0f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(
        TEXT("FollowCamera")
    );

    FollowCamera->SetupAttachment(
        CameraBoom,
        USpringArmComponent::SocketName
    );

    FollowCamera->bUsePawnControlRotation = false;
    FollowCamera->SetAutoActivate(true);
    FollowCamera->SetActive(true);

    // ─────────────────────────────────────────────
    // 1인칭 카메라

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(
        TEXT("FirstPersonCamera")
    );

    // 스켈레톤의 head 본에 만든 CameraSocket에 부착
    FirstPersonCamera->SetupAttachment(
        GetMesh(),
        TEXT("CameraSocket")
    );

    // 위치와 회전은 CameraSocket이 담당
    FirstPersonCamera->SetRelativeLocation(FVector::ZeroVector);
    FirstPersonCamera->SetRelativeRotation(FRotator::ZeroRotator);

    FirstPersonCamera->bUsePawnControlRotation = true;

    // 기본 시작은 3인칭
    FirstPersonCamera->SetAutoActivate(false);
    FirstPersonCamera->SetActive(false);
}

void ANCPlayerCharacter::InitComponents()
{
    InteractionComponent = CreateDefaultSubobject<UNCInteractionComponent>(TEXT("InteractionComponent"));
    LocomotionComponent = CreateDefaultSubobject<UNCLocomotionComponent>(TEXT("LocomotionComponent"));
    CombatComponent = CreateDefaultSubobject<UNCCombatComponent>(TEXT("CombatComponent"));
    AssassinationComponent = CreateDefaultSubobject<UNCAssassinationComponent>(TEXT("AssassinationComponent")); //헌호수정
    EquipmentComponent = CreateDefaultSubobject<UNCEquipmentComponent>(TEXT("EquipmentComponent"));
    RifleComponent     = CreateDefaultSubobject<UNCRifleComponent>(TEXT("RifleComponent"));
    ShotgunComponent   = CreateDefaultSubobject<UNCShotgunComponent>(TEXT("ShotgunComponent"));
    PistolComponent    = CreateDefaultSubobject<UNCPistolComponent>(TEXT("PistolComponent"));

    // 헌호수정 - 플래시라이트 컴포넌트 생성 및 소켓에 부착
    FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
    FlashlightMesh->SetupAttachment(GetMesh(), TEXT("Flashlight_Socket"));

    FlashlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightLight"));
    FlashlightLight->SetupAttachment(FlashlightMesh);
    FlashlightLight->SetVisibility(false); //헌호수정 - 기본 꺼짐
    FlashlightLight->SetCastShadows(false); //헌호수정 - 캐릭터 얼굴 통과 그림자 방지

    //헌호수정 - 디비전 스타일 백팩 체력바 (3D 위젯)
    // 애니메이션(뛰기/공격)에 안 흔들리도록 본이 아닌 캡슐(루트)에 부착 → 캐릭터 이동/회전만 따라감
    BackpackHPWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("BackpackHPWidget"));
    BackpackHPWidget->SetupAttachment(GetCapsuleComponent());        // 루트에 부착 (흔들림 방지)
    BackpackHPWidget->SetRelativeLocation(FVector(-30.f, 0.f, 40.f)); // 등 뒤 위쪽 (BP에서 조정)
    BackpackHPWidget->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));  // 등 뒤 방향 (BP에서 조정)
    BackpackHPWidget->SetWidgetSpace(EWidgetSpace::World);           // 3D 월드 공간
    BackpackHPWidget->SetDrawSize(FVector2D(120.f, 20.f));           // 심플 게이지 크기
    BackpackHPWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // 시환 추가 - 좀비 공격 슬롯 추가
    AttackSlotComponent = CreateDefaultSubobject<UVGAttackSlotComponent>(TEXT("AttackSlotComponent"));
}

void ANCPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    bIsFirstPerson = false;

    if (FollowCamera)
    {
        FollowCamera->SetActive(true);
    }

    if (FirstPersonCamera)
    {
        FirstPersonCamera->SetActive(false);
    }

    // EquipmentComponent에 총기 타입별 컴포넌트 등록
    if (EquipmentComponent)
    {
        EquipmentComponent->WeaponComponents.Add(NCGun::Type_Rifle,   RifleComponent);
        EquipmentComponent->WeaponComponents.Add(NCGun::Type_Shotgun, ShotgunComponent);
        EquipmentComponent->WeaponComponents.Add(NCGun::Type_Pistol,  PistolComponent);
    }

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
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            UVGPlayerAttributeSet::GetHealthAttribute())
            .AddUObject(this, &ANCPlayerCharacter::HandleHealthChanged);
    }

    //헌호수정 - 백팩 체력바 초기값 표시 (풀피)
    UpdateBackpackHP();

    //헌호수정 - 정조준 HUD 위젯 한 번 생성 후 숨김 (로컬 플레이어만)
    if (IsLocallyControlled() && ADSHUDWidgetClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            ADSHUDWidget = CreateWidget<UNCADSHUD>(PC, ADSHUDWidgetClass);
            if (ADSHUDWidget)
            {
                ADSHUDWidget->AddToViewport();
                ADSHUDWidget->SetVisibility(ESlateVisibility::Collapsed); // 기본 숨김
            }
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

void ANCPlayerCharacter::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    //헌호수정 - 체력 증감 모두 백팩 체력바에 반영 (아래 early return 전에 갱신)
    UpdateBackpackHP();

    if (Data.NewValue >= Data.OldValue) return;
    if (Data.NewValue <= 0.f) return;
}

//헌호수정 - 백팩 3D 체력바 갱신 (모든 클라이언트에서 호출됨 → 상대 플레이어도 보임)
void ANCPlayerCharacter::UpdateBackpackHP()
{
    if (!BackpackHPWidget)
    {
        return;
    }

    if (UNCBackpackHUD* HUD = Cast<UNCBackpackHUD>(BackpackHPWidget->GetUserWidgetObject()))
    {
        HUD->RefreshAll(this);
    }
}

//헌호수정 - 정조준 여부에 따라 백팩 UI ↔ 정조준 UI 전환 + 데이터 갱신
void ANCPlayerCharacter::UpdateWeaponHUDs()
{
    // 현재 ADS 상태 확인
    bool bIsADS = false;
    if (UNCEquipmentComponent* Equip = GetEquipmentComponent())
    {
        bIsADS = Equip->IsADS();
    }

    // 상태가 바뀐 순간에만 UI 전환 (매 프레임 SetVisibility 낭비 방지)
    if (bIsADS != bWasADS)
    {
        bWasADS = bIsADS;

        // 백팩 UI: ADS 중이면 숨김
        if (BackpackHPWidget)
        {
            BackpackHPWidget->SetVisibility(!bIsADS);
        }

        // 정조준 UI: ADS 중이면 표시 (로컬 플레이어만 생성돼있음)
        if (ADSHUDWidget)
        {
            ADSHUDWidget->SetVisibility(bIsADS ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
        }
    }

    // 켜져 있는 쪽만 데이터 갱신
    if (bIsADS)
    {
        if (ADSHUDWidget)
        {
            ADSHUDWidget->RefreshAll(this);
        }
    }
    else
    {
        UpdateBackpackHP();
    }
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
        // 헌호수정 - 서버에서도 크라우치 스프린트 중 일어서면 조그로 전환
        if (CurrentGaitTag == NCCharacter::CrouchSprint)
        {
            CurrentGaitTag = NCCharacter::Jog;
            if (LocomotionComponent)
                LocomotionComponent->StopStaminaDrain();
        }
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
    if (UNCGunComponent* GunComp = GetGunComponent())
    {
        if (GunComp->IsADS())
        {
            return;
        }
    }

    // 헌호수정 - 스프린트 잠금 중이면 속도 변경도 막음
    if (LocomotionComponent && LocomotionComponent->IsSprintLocked()) return;

    if (LocomotionComponent)
        LocomotionComponent->StartStaminaDrain();

    CurrentGaitTag = (CurrentStanceTag == NCCharacter::Crouch)
        ? NCCharacter::CrouchSprint
        : NCCharacter::Sprint;

    if (LocomotionComponent)
        LocomotionComponent->SetGaitTag(CurrentGaitTag);

    Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::StopSprint()
{
    // 헌호수정
    if (LocomotionComponent)
        LocomotionComponent->StopStaminaDrain();

    // 헌호수정 - Gait는 항상 Jog로 복귀 (앉은 상태라도 Jog 유지, 속도는 ApplyMovementSpeed에서 Crouch 행 사용)
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

        // 헌호수정 - 크라우치 스프린트 중 일어서면 조그로 전환 + 스태미나 드레인 중지
        if (CurrentGaitTag == NCCharacter::CrouchSprint)
        {
            CurrentGaitTag = NCCharacter::Jog;
            if (LocomotionComponent)
                LocomotionComponent->StopStaminaDrain();
            // 헌호수정 - 서버에도 Gait 변경 알림 (서버가 클라 속도 덮어쓰는 것 방지)
            Server_SetGait(CurrentGaitTag);
        }

        if (LocomotionComponent)
        {
            // 헌호수정 - StanceTag 먼저 Stand로 변경 후 속도 재적용
            LocomotionComponent->SetStanceTag(NCCharacter::Stand);
            LocomotionComponent->SetGaitTag(CurrentGaitTag);
        }
    }
    else
    {
        // 헌호수정 - 앉을 때 스프린트/크라우치스프린트 중이면 스태미나 드레인 중지
        if ((CurrentGaitTag == NCCharacter::Sprint || CurrentGaitTag == NCCharacter::CrouchSprint)
            && LocomotionComponent)
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
    ClearDeathRelatedTimers();

    if (PlayerInventoryRef)
    {
        PlayerInventoryRef->bHasPendingConsumable = false;
        PlayerInventoryRef->PendingConsumableData = FConsumableItemData();
        PlayerInventoryRef->PendingReEquipGunSlot = ENCGunSlot::None;
        PlayerInventoryRef->bPendingReEquipMelee = false;
        PlayerInventoryRef->PendingReEquipMeleeInstance = FNCWeaponInstance();
        PlayerInventoryRef->PendingUseItemTag = FGameplayTag::EmptyTag;
        PlayerInventoryRef->OnItemUsed.RemoveDynamic(this, &ANCPlayerCharacter::OnItemUsed);
    }

    if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &ANCPlayerCharacter::OnConsumableMontageEnded);
    }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        ASC->AddLooseGameplayTag(NCCharacter::Dead);
        ASC->RemoveLooseGameplayTag(NCWeapon::Action_Attacking);
        ASC->RemoveLooseGameplayTag(NCWeapon::Action_UsingItem);
    }

    if (EquipmentComponent)
    {
        EquipmentComponent->StopFire();
        EquipmentComponent->StopADS();
        
        if (UNCGunComponent* ActiveWeapon = EquipmentComponent->GetActiveWeapon())
        {
            ActiveWeapon->EndInfiniteAmmo();
            ActiveWeapon->EndDamageBoost();
        }
    }

    if (InteractionComponent)
    {
        InteractionComponent->StopInteraction();
    }

    if (LocomotionComponent)
    {
        LocomotionComponent->ClearAllStaminaTimers();
        LocomotionComponent->EndSpeedBoost();
    }
    
    EndInvincibility();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }

    bIsStunned = false;
    bCameraShaking = false;

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
    }

    Multicast_OnDead();

    const float MontageLength = DeathMontage ? DeathMontage->GetPlayLength() : 0.f;
    const float DestroyDelay = MontageLength + 5.f;

    if (HasAuthority())
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                DeathTimerHandle,
                this,
                &ANCPlayerCharacter::CleanupBeforeDeathDestroy,
                DestroyDelay,
                false
            );
        }
    }
}

void ANCPlayerCharacter::Multicast_OnDead_Implementation() //헌호수정
{
    APlayerController* PC = Cast<APlayerController>(GetController());

    // 헌호수정 - 입력 차단 (로컬 컨트롤러에만 의미 있음)
    if (PC)
        PC->DisableInput(PC);

    // 헌호수정 - 사망 몽타지 재생 (모든 클라이언트)
    if (DeathMontage)
        PlayAnimMontage(DeathMontage);

    // 사망 결과 UI - 본인 화면에만 표시 (탈출 실패, bSuccess 기본값 false)
    if (IsLocallyControlled() && PC && EscapeResultWidgetClass)
    {
        if (UUserWidget* ResultWidget = CreateWidget<UUserWidget>(PC, EscapeResultWidgetClass))
        {
            ResultWidget->AddToViewport();

            FInputModeUIOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
        }
    }
}

void ANCPlayerCharacter::OnItemUsed(FGameplayTag UsedItemTag)
{
    UAnimMontage* MontageToPlay = nullptr;

    if (UsedItemTag.MatchesTag(NCItemTag::Heal) && HealItemMontage)
        MontageToPlay = HealItemMontage;
    else if (UsedItemTag.MatchesTag(NCItemTag::Food) && FoodItemMontage)
        MontageToPlay = FoodItemMontage;
    else if (UseItemMontage)
        MontageToPlay = UseItemMontage;

    if (MontageToPlay)
    {
        PlayAnimMontage(MontageToPlay);

        if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            AnimInst->OnMontageEnded.RemoveDynamic(this, &ANCPlayerCharacter::OnConsumableMontageEnded);
            AnimInst->OnMontageEnded.AddDynamic(this, &ANCPlayerCharacter::OnConsumableMontageEnded);
        }
    }
    else
    {
        OnUseItemMontageEnded();
    }
}

void ANCPlayerCharacter::OnConsumableMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != HealItemMontage && Montage != FoodItemMontage && Montage != UseItemMontage)
        return;

    if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        AnimInst->OnMontageEnded.RemoveDynamic(this, &ANCPlayerCharacter::OnConsumableMontageEnded);

    //헌호수정 - 피격 등으로 중단되면 소모품 효과 적용 취소 (무기 복원은 그대로 진행)
    if (bInterrupted && PlayerInventoryRef)
    {
        PlayerInventoryRef->bHasPendingConsumable = false;
    }

    OnUseItemMontageEnded();
}

void ANCPlayerCharacter::OnUseItemMontageEnded()
{
    if (!PlayerInventoryRef) return;

    // 소모품 효과 적용
    if (PlayerInventoryRef->bHasPendingConsumable)
    {
        FConsumableItemData& Data = PlayerInventoryRef->PendingConsumableData;
        PlayerInventoryRef->bHasPendingConsumable = false;

        if (UAbilitySystemComponent* NCASC = GetAbilitySystemComponent())
        {
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
    }

    // 소모품 사용 전 장착중이던 무기 복원 (효과 적용 성공 여부와 무관하게 항상 실행)
    const ENCGunSlot ReEquipGunSlot = PlayerInventoryRef->PendingReEquipGunSlot;
    PlayerInventoryRef->PendingReEquipGunSlot = ENCGunSlot::None;
    if (ReEquipGunSlot != ENCGunSlot::None)
    {
        if (UNCEquipmentComponent* EquipComp = GetEquipmentComponent())
        {
            EquipComp->SelectSlot(ReEquipGunSlot);
        }
    }

    const bool bReEquipMelee = PlayerInventoryRef->bPendingReEquipMelee;
    PlayerInventoryRef->bPendingReEquipMelee = false;
    if (bReEquipMelee)
    {
        if (UNCCombatComponent* Combat = FindComponentByClass<UNCCombatComponent>())
        {
            Combat->EquipWeapon(PlayerInventoryRef->PendingReEquipMeleeInstance);
        }
    }

    // 사용 중 차단 태그 해제
    if (UAbilitySystemComponent* NCASC = GetAbilitySystemComponent())
    {
        NCASC->RemoveLooseGameplayTag(NCWeapon::Action_UsingItem);
    }
}

//H
void ANCPlayerCharacter::HandleHitReact(AActor* Attacker)
{

    if (!Attacker)
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();

    if (Now - LastHitReactTime < HitReactCooldown)
    {
        return;
    }

    LastHitReactTime = Now;

    const FVector ToAttacker =
        (Attacker->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

    const float ForwardDot =
        FVector::DotProduct(GetActorForwardVector(), ToAttacker);

    const float RightDot =
        FVector::DotProduct(GetActorRightVector(), ToAttacker);

    UAnimMontage* SelectedMontage = nullptr;

    if (ForwardDot > 0.5f)
    {
        SelectedMontage = HitReactFrontMontage;
    }
    else if (ForwardDot < -0.5f)
    {
        SelectedMontage = HitReactBackMontage;
    }
    else if (RightDot > 0.f)
    {
        SelectedMontage = HitReactRightMontage;
    }
    else
    {
        SelectedMontage = HitReactLeftMontage;
    }

    if (SelectedMontage)
    {
        Multicast_PlayHitReactMontage(SelectedMontage);
    }
    else
    {
    }

    if (TakeDamageShakeClass && !bCameraShaking)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            PC->ClientStartCameraShake(TakeDamageShakeClass);

            bCameraShaking = true;

            GetWorld()->GetTimerManager().SetTimer(
                ShakeTimerHandle,
                [this]() { bCameraShaking = false; },
                0.6f,
                false);
        }
    }

    //헌호수정 - 피격 시 스턴 + 행동 취소
    ApplyStun();
}

//헌호수정 - 스턴 적용: 하던 행동 취소 + 이동/시점 입력 잠금
void ANCPlayerCharacter::ApplyStun()
{
    if (StateTags.HasTagExact(NCCharacter::Dead))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    if (bIsStunned)
    {
        World->GetTimerManager().ClearTimer(StunTimerHandle);
        World->GetTimerManager().SetTimer(
            StunTimerHandle,
            this,
            &ANCPlayerCharacter::EndStun,
            StunDuration,
            false
        );
        return;
    }

    bIsStunned = true;

    if (UNCCombatComponent* Combat = GetCombatComponent())
    {
        if (UAnimMontage* AtkMontage = Combat->GetLastPlayedAttackMontage())
        {
            StopAnimMontage(AtkMontage);
        }
    }

    if (HealItemMontage) StopAnimMontage(HealItemMontage);
    if (FoodItemMontage) StopAnimMontage(FoodItemMontage);
    if (UseItemMontage)  StopAnimMontage(UseItemMontage);

    if (EquipmentComponent)
    {
        EquipmentComponent->StopFire();
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }

    World->GetTimerManager().SetTimer(
        StunTimerHandle,
        this,
        &ANCPlayerCharacter::EndStun,
        StunDuration,
        false
    );
}

//헌호수정 - 스턴 해제: 입력 잠금 복구
void ANCPlayerCharacter::EndStun()
{
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        World->GetTimerManager().ClearTimer(StunTimerHandle);
    }

    StunTimerHandle.Invalidate();
    bIsStunned = false;

    if (StateTags.HasTagExact(NCCharacter::Dead))
    {
        return;
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
    }
}

float ANCPlayerCharacter::GetFootstepVolumeMultiplier() const
{
    if (CurrentGaitTag.MatchesTagExact(NCCharacter::Sprint))
    {
        return 1.0f;
    }

    if (CurrentGaitTag.MatchesTagExact(NCCharacter::CrouchSprint))
    {
        return 0.5f;
    }

    if (CurrentStanceTag.MatchesTagExact(NCCharacter::Crouch))
    {
        return 0.3f;
    }

    if (CurrentGaitTag.MatchesTagExact(NCCharacter::Jog))
    {
        return 0.7f;
    }

    return 0.7f;
}

void ANCPlayerCharacter::TryAssassinate() //헌호수정 - 암살 컴포넌트에 위임
{
    if (!HasAuthority())
    {
        Server_TryAssassinate();
        return;
    }

    if (AssassinationComponent)
        AssassinationComponent->TryAssassinate();
}

void ANCPlayerCharacter::Server_TryAssassinate_Implementation()
{
    TryAssassinate();
}

void ANCPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANCPlayerCharacter, bFlashlightOn); //헌호수정
}

// 헌호수정 - T키 입력 시 호출
void ANCPlayerCharacter::ToggleFlashlight()
{
    Server_ToggleFlashlight();
}

void ANCPlayerCharacter::ApplyFirstPersonWeaponCameraOffset(
    ENCGunSlot WeaponSlot)
{
    if (!FirstPersonCamera)
    {
        return;
    }

    FVector TargetOffset = FirstPersonDefaultCameraOffset;
    FRotator TargetRotation = FirstPersonDefaultCameraRotation;

    switch (WeaponSlot)
    {
    case ENCGunSlot::Shotgun:
        TargetOffset += FirstPersonShotgunCameraOffset;
        TargetRotation += FirstPersonShotgunCameraRotation;
        break;

    case ENCGunSlot::Rifle:
        TargetOffset += FirstPersonRifleCameraOffset;
        TargetRotation += FirstPersonRifleCameraRotation;
        break;

    case ENCGunSlot::Sidearm:
        TargetOffset += FirstPersonSidearmCameraOffset;
        TargetRotation += FirstPersonSidearmCameraRotation;
        break;

    case ENCGunSlot::None:
    default:
        break;
    }

    FirstPersonCamera->SetRelativeLocation(TargetOffset);
    FirstPersonCamera->SetRelativeRotation(TargetRotation);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("First Person Camera Applied - Slot: %d, Offset: %s, Rotation: %s"),
        static_cast<int32>(WeaponSlot),
        *TargetOffset.ToString(),
        *TargetRotation.ToString()
    );
}

void ANCPlayerCharacter::ToggleView()
{
    if (!IsLocallyControlled())
    {
        return;
    }

    if (!FollowCamera || !FirstPersonCamera)
    {
        return;
    }

    bIsFirstPerson = !bIsFirstPerson;

    if (bIsFirstPerson)
    {
        FollowCamera->Deactivate();
        FirstPersonCamera->Activate();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Camera View Changed: First Person")
        );
    }
    else
    {
        FirstPersonCamera->Deactivate();
        FollowCamera->Activate();

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Camera View Changed: Third Person")
        );
    }
}

// 헌호수정 - 서버에서 상태 토글
void ANCPlayerCharacter::Server_ToggleFlashlight_Implementation()
{
    bFlashlightOn = !bFlashlightOn;
    ApplyFlashlightState(); // 서버 적용
}

// 헌호수정 - 클라이언트 복제 콜백
void ANCPlayerCharacter::OnRep_bFlashlightOn()
{
    ApplyFlashlightState();
}

// 헌호수정 - 실제 켜고 끄기 (서버/클라 공통)
void ANCPlayerCharacter::ApplyFlashlightState()
{
    if (FlashlightLight)
        FlashlightLight->SetVisibility(bFlashlightOn);
}

void ANCPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //헌호수정 - 백팩/ADS HUD 실시간 갱신 + 전환
    UpdateWeaponHUDs();

    //헌호수정 - 플래시라이트 빛 방향만 컨트롤러(카메라) 조준 방향으로 고정
    // (손전등 메시 모델은 어깨에 자연스럽게 유지, 빛만 안 흔들리게)
    if (bFlashlightOn && FlashlightLight)
    {
        if (AController* FlashCtrl = GetController())
        {
            FlashlightLight->SetWorldRotation(FlashCtrl->GetControlRotation());
        }
    }
    
    // 시환 추가 - 카메라 접근 시, 플레이어 투명화
    //const FVector ArmOrigin = CameraBoom->GetComponentLocation();
    //const FVector CameraSocketLocation = CameraBoom->GetSocketLocation(USpringArmComponent::SocketName);
    //const float CurrentArmLength = FVector::Dist(ArmOrigin, CameraSocketLocation);
    //const bool bCameraTooClose = CurrentArmLength < 200.f;
    //GetMesh()->SetVisibility(!bCameraTooClose, true);
    
    if (FlashlightLight)
    {
        FlashlightLight->SetVisibility(bFlashlightOn);
    }

    AController* OwnerController = GetController();
    if (!OwnerController)
    {
        return;
    }

    const FRotator ControlRot = OwnerController->GetControlRotation();
    const FRotator TargetRot = FRotator(0.f, ControlRot.Yaw, 0.f);

    SetActorRotation(TargetRot);
}

void ANCPlayerCharacter::SetAimRotationMode(bool bEnable)
{
    bAimRotationMode = bEnable;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = false;
        Move->bUseControllerDesiredRotation = false;
    }

    bUseControllerRotationYaw = false;

    if (CameraBoom)
    {
        CameraBoom->bEnableCameraLag = !bEnable;
    }
}

void ANCPlayerCharacter::Multicast_PlayHitReactMontage_Implementation(UAnimMontage* MontageToPlay)
{
    if (MontageToPlay)
    {
        PlayAnimMontage(MontageToPlay);
    }
}

void ANCPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearDeathRelatedTimers();

    if (PlayerInventoryRef)
    {
        PlayerInventoryRef->OnItemUsed.RemoveDynamic(this, &ANCPlayerCharacter::OnItemUsed);
    }

    if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &ANCPlayerCharacter::OnConsumableMontageEnded);
    }

    if (ADSHUDWidget)
    {
        ADSHUDWidget->RemoveFromParent();
        ADSHUDWidget = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

void ANCPlayerCharacter::ClearDeathRelatedTimers()
{
    UWorld* World = GetWorld();

    if (!IsValid(World))
    {
        DeathTimerHandle.Invalidate();
        StunTimerHandle.Invalidate();
        ShakeTimerHandle.Invalidate();
        ComboResetTimerHandle.Invalidate();
        return;
    }

    World->GetTimerManager().ClearTimer(DeathTimerHandle);
    World->GetTimerManager().ClearTimer(StunTimerHandle);
    World->GetTimerManager().ClearTimer(ShakeTimerHandle);
    World->GetTimerManager().ClearTimer(ComboResetTimerHandle);

    DeathTimerHandle.Invalidate();
    StunTimerHandle.Invalidate();
    ShakeTimerHandle.Invalidate();
    ComboResetTimerHandle.Invalidate();
    
    // 시환 추가 - 사망 시 특수 아이템 버프 정리
    if (LocomotionComponent)
    {
        LocomotionComponent->EndSpeedBoost();
    }

    if (EquipmentComponent)
    {
        for (const TPair<FGameplayTag, TObjectPtr<UNCGunComponent>>& Pair : EquipmentComponent->WeaponComponents)
        {
            if (UNCGunComponent* Gun = Pair.Value)
            {
                Gun->EndInfiniteAmmo();
                Gun->EndDamageBoost();
            }
        }
    }
}

void ANCPlayerCharacter::CleanupBeforeDeathDestroy()
{
    ClearDeathRelatedTimers();

    if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInst->OnMontageEnded.RemoveDynamic(this, &ANCPlayerCharacter::OnConsumableMontageEnded);
        AnimInst->StopAllMontages(0.1f);
    }

    if (EquipmentComponent)
    {
        EquipmentComponent->StopFire();
        EquipmentComponent->StopADS();
    }

    if (LocomotionComponent)
    {
        LocomotionComponent->ClearAllStaminaTimers();
    }

    if (InteractionComponent)
    {
        InteractionComponent->StopInteraction();
    }

    Destroy();
}

void ANCPlayerCharacter::AddKillCombo()
{
    // 연속 처치 횟수 증가
    CurrentComboCount++;

    // 현재 콤보 배율
    const float ComboMultiplier = GetCurrentComboMultiplier();

    // 콤보 이름
    FString ComboText;

    switch (CurrentComboCount)
    {
    case 1:
        ComboText = TEXT("KILL");
        break;

    case 2:
        ComboText = TEXT("DOUBLE KILL");
        break;

    case 3:
        ComboText = TEXT("TRIPLE KILL");
        break;

    case 4:
        ComboText = TEXT("QUAD KILL");
        break;

    default:
        ComboText = TEXT("MASSACRE");
        break;
    }

    // 화면 디버그 출력
    if (GEngine && IsLocallyControlled())
    {
        GEngine->AddOnScreenDebugMessage(
            2001,
            2.0f,
            FColor::Red,
            FString::Printf(
                TEXT("%s | %d COMBO | x%.1f"),
                *ComboText,
                CurrentComboCount,
                ComboMultiplier
            )
        );
    }

    // 기존 콤보 종료 타이머 초기화
    GetWorldTimerManager().ClearTimer(ComboResetTimerHandle);

    // 마지막 처치 이후 ComboResetTime이 지나면 콤보 종료
    if (ComboResetTime > 0.0f)
    {
        GetWorldTimerManager().SetTimer(
            ComboResetTimerHandle,
            this,
            &ANCPlayerCharacter::ResetKillCombo,
            ComboResetTime,
            false
        );
    }

    // 출력 로그
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s | Kill Combo: %d | Multiplier: x%.1f"),
        *ComboText,
        CurrentComboCount,
        ComboMultiplier
    );
}

void ANCPlayerCharacter::ResetKillCombo()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Kill Combo End: %d Combo"),
        CurrentComboCount
    );

    OnComboReset.Broadcast();
    CurrentComboCount = 0;
}

float ANCPlayerCharacter::GetCurrentComboMultiplier() const
{
    if (CurrentComboCount <= 1)
    {
        return 1.0f;
    }

    if (CurrentComboCount == 2)
    {
        return 1.1f;
    }

    if (CurrentComboCount == 3)
    {
        return 1.2f;
    }

    if (CurrentComboCount == 4)
    {
        return 1.35f;
    }

    // 5킬 이상
    return 1.5f;
}