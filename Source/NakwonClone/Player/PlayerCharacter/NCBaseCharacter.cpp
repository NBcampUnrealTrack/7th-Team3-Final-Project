#include "NCBaseCharacter.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Player/PlayerComponent/UNCStatComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

ANCBaseCharacter::ANCBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    CurrentActionTag = FGameplayTag::EmptyTag;

    //TODO: GAS 완전 전환 후 제거
    StatComponent = CreateDefaultSubobject<UNCStatComponent>(TEXT("StatComponent"));

    //GAS
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    
    PlayerAttributeSet = CreateDefaultSubobject<UVGPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

UAbilitySystemComponent* ANCBaseCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

UVGPlayerAttributeSet* ANCBaseCharacter::GetPlayerAttributeSet() const
{
    return PlayerAttributeSet;
}

void ANCBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}

void ANCBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANCBaseCharacter, CurrentGaitTag);
    DOREPLIFETIME(ANCBaseCharacter, CurrentStanceTag);
    DOREPLIFETIME(ANCBaseCharacter, CurrentActionTag);
}

void ANCBaseCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    CurrentActionTag = NCCharacter::Landed;
    LandVelocity = GetCharacterMovement()->Velocity;

    GetWorldTimerManager().SetTimer(
        LandingTimerHandle,
        this,
        &ANCBaseCharacter::ResetLandingState,
        0.15f,
        false
    );
}


void ANCBaseCharacter::ResetLandingState()
{
    CurrentActionTag = FGameplayTag::EmptyTag;
}

void ANCBaseCharacter::OnDead()
{
    // 헌호수정 - 사망 시 착지 타이머 정리 (파괴된 액터 접근 방지)
    GetWorldTimerManager().ClearTimer(LandingTimerHandle);
    
    // 시환 추가 - 사망 시 무적 효과 즉시 해제
    EndInvincibility();
}

void ANCBaseCharacter::ActivateInvincibility(float Duration)
{
    if (UVGPlayerAttributeSet* AttrSet = GetPlayerAttributeSet())
    {
        AttrSet->SetInvincible(true);
    }

    GetWorldTimerManager().SetTimer(
        InvincibilityTimerHandle, this, &ANCBaseCharacter::EndInvincibility, Duration, false);
}

void ANCBaseCharacter::EndInvincibility()
{
    GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimerHandle);

    if (UVGPlayerAttributeSet* AttrSet = GetPlayerAttributeSet())
    {
        AttrSet->SetInvincible(false);
    }
}