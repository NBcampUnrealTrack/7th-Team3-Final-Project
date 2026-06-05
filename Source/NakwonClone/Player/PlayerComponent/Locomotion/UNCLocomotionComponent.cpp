#include "UNCLocomotionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "NakwonClone/Player/PlayerData/NCPlayerMovementData.h"

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
}

void UNCLocomotionComponent::SetGaitTag(FGameplayTag NewGaitTag)
{
	if (CurrentGaitTag == NewGaitTag) return; //같은 상태면 무시

	CurrentGaitTag = NewGaitTag;
	ApplyMovementSpeed();
}

void UNCLocomotionComponent::SetStanceTag(FGameplayTag NewStanceTag)
{
	if (CurrentStanceTag == NewStanceTag) return;

	CurrentStanceTag = NewStanceTag;
}

void UNCLocomotionComponent::ApplyMovementSpeed()
{
	if (!OwnerCharacter || !MovementComponent) return;

	if (!MovementDataTable) return;

	FName RowName = CurrentGaitTag.GetTagName();
	FNcPlayerMovementData* Data = MovementDataTable->FindRow
		<FNcPlayerMovementData>(RowName, TEXT("Locomotion"));

	if (Data)
	{
		MovementComponent->MaxWalkSpeed = Data->MovementSpeed;
		MovementComponent->MaxAcceleration = Data->MaxAcceleration;
	}
}