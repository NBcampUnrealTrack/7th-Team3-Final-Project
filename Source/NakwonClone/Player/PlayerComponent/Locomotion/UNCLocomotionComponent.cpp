#include "UNCLocomotionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "NakwonClone/Player/PlayerData/NCPlayerMovementData.h"
//헌호수정
#include "AbilitySystemComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGPlayerAttributeSet.h"

UNCLocomotionComponent::UNCLocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNCLocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

	//BeginPlay에서 한번만 캐싱 (매 틱 캐스팅 방지)
	OwnerCharacter = Cast<ANCBaseCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}

	//기본 상태 설정
	CurrentGaitTag = NCCharacter::Jog;
	CurrentStanceTag = NCCharacter::Stand;

	//H
	ApplyMovementSpeed();
}

void UNCLocomotionComponent::SetGaitTag(FGameplayTag NewGaitTag)
{
	CurrentGaitTag = NewGaitTag;
	ApplyMovementSpeed();
}

void UNCLocomotionComponent::SetStanceTag(FGameplayTag NewStanceTag)
{
	if (CurrentStanceTag == NewStanceTag) return;

	CurrentStanceTag = NewStanceTag;
	// 헌호수정 - 크라우치 전환 시 속도 재적용
	ApplyMovementSpeed();
}

void UNCLocomotionComponent::ApplyMovementSpeed()
{
	if (!OwnerCharacter || !MovementComponent) return;

	if (!MovementDataTable) return;

	// 헌호수정 - 크라우치 스프린트면 CrouchSprint 행, 크라우치면 Crouch 행, 아니면 Gait 행
	FName RowName;
	if (CurrentStanceTag == NCCharacter::Crouch)
	{
		RowName = (CurrentGaitTag == NCCharacter::CrouchSprint)
			? NCCharacter::CrouchSprint.GetTag().GetTagName()
			: NCCharacter::Crouch.GetTag().GetTagName();
	}
	else
	{
		RowName = CurrentGaitTag.GetTagName();
	}

	FNcPlayerMovementData* Data = MovementDataTable->FindRow
		<FNcPlayerMovementData>(RowName, TEXT("Locomotion"));

	if (Data)
	{
		MovementComponent->MaxWalkSpeed = Data->MovementSpeed;
		MovementComponent->MaxAcceleration = Data->MaxAcceleration;
		// 헌호수정 - 자연스러운 이동감을 위한 감속/마찰/회전 적용
		MovementComponent->BrakingDecelerationWalking = Data->BrakingDeceleration;
		MovementComponent->BrakingFrictionFactor = Data->BrakingFrictionFactor;
		MovementComponent->RotationRate = FRotator(0.f, Data->RotationRate, 0.f);
		// 헌호수정 - 크라우치 전용 속도도 같이 적용
		if (CurrentStanceTag == NCCharacter::Crouch)
			MovementComponent->MaxWalkSpeedCrouched = Data->MovementSpeed;

	}
}

// 헌호수정 - 사망 시 모든 스태미나 타이머 정리
void UNCLocomotionComponent::ClearAllStaminaTimers()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaDrainHandle);
	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);
	bSprintLocked = false;
}

//헌호수정 - 스태미나 드레인 시작 (Sprint 시작 시 호출)
void UNCLocomotionComponent::StartStaminaDrain()
{
	if (bSprintLocked) return;

	GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);
	GetWorld()->GetTimerManager().SetTimer(
		StaminaDrainHandle, this, &UNCLocomotionComponent::DrainStamina, 0.1f, true);
}

//헌호수정 - 스태미나 드레인 중지 (Sprint 종료 시 호출)
void UNCLocomotionComponent::StopStaminaDrain()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaDrainHandle);
	GetWorld()->GetTimerManager().SetTimer(
		StaminaRegenHandle, this, &UNCLocomotionComponent::RegenStamina, 0.1f, true);
}

void UNCLocomotionComponent::DrainStamina()
{
	if (!OwnerCharacter) return;

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;

	const float Current = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetStaminaAttribute());
	const float NewValue = FMath::Max(Current - StaminaDrainRate * 0.1f, 0.f);
	ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetStaminaAttribute(), NewValue);

	if (NewValue <= 0.f)
		OnStaminaEmpty();
}

void UNCLocomotionComponent::RegenStamina()
{
	if (!OwnerCharacter) return;

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;

	const float Current = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetStaminaAttribute());
	const float Max = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxStaminaAttribute());
	const float NewValue = FMath::Min(Current + StaminaRegenRate * 0.1f, Max);
	ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetStaminaAttribute(), NewValue);

	// 헌호수정 - 임계값 도달 시 스프린트 잠금만 해제, 리젠은 계속
	if (bSprintLocked && NewValue >= StaminaRegenThreshold)
	{
		bSprintLocked = false;
	}

	// 스태미나 꽉 차면 리젠 타이머 종료
	if (NewValue >= Max)
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRegenHandle);
	}
}

void UNCLocomotionComponent::OnStaminaEmpty()
{
	bSprintLocked = true;
	GetWorld()->GetTimerManager().ClearTimer(StaminaDrainHandle);
	GetWorld()->GetTimerManager().SetTimer(
		StaminaRegenHandle, this, &UNCLocomotionComponent::RegenStamina, 0.1f, true);

	SetGaitTag(NCCharacter::Jog);
	if (OwnerCharacter)
		OwnerCharacter->CurrentGaitTag = NCCharacter::Jog;
}