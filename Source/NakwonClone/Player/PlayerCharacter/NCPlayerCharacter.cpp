#include "NCPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Weapon/NCWeaponBase.h"
#include "NakwonClone/GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"

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
  
    PlayerInventory = CreateDefaultSubobject<UNCPlayerInventoryComponent>(TEXT("PlayerInventory"));
    
}

void ANCPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    //AttributeSet 초기화
    if (AbilitySystemComponent)
    {
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

void ANCPlayerCharacter::EquipWeapon(TSubclassOf<ANCWeaponBase> WeaponClass)
{
    if (!HasAuthority()) return;
    if (!WeaponClass) return;

    //기존 무기 해제
    if (CurrentWeapon)
    {
        CurrentWeapon->DetachFromCharacter();
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }

    //새 무기 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    CurrentWeapon = GetWorld()->SpawnActor<ANCWeaponBase>(
        WeaponClass, FTransform::Identity, SpawnParams);

    if (CurrentWeapon)
    {
        CurrentWeapon->AttachToCharacter(GetMesh(), FName("weapon_r"));
    }
}

void ANCPlayerCharacter::UnEquipWeapon()
{
    if (!HasAuthority()) return;
    if (!CurrentWeapon) return;

    CurrentWeapon->DetachFromCharacter();
    CurrentWeapon->Destroy();
    CurrentWeapon = nullptr;
}