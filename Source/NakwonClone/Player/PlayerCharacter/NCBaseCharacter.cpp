#include "NCBaseCharacter.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Player/PlayerData/NCPlayerMovementData.h"

ANCBaseCharacter::ANCBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	
	CurrentGaitTag = NCCharacter::Jog;
	CurrentStanceTag = NCCharacter::Stand;
	CurrentActionTag = FGameplayTag::EmptyTag;
}

void ANCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

void ANCBaseCharacter::ApplyMovementData(FGameplayTag StateTag)
{
	if (!MovementDataTable)
	{
		return;
	}
	
	FName RowName = StateTag.GetTagName();
	FString ContextString = TEXT("Movement Data");
	
	FNcPlayerMovementData* RowData = MovementDataTable->FindRow<FNcPlayerMovementData>(RowName, ContextString);
	
	if (RowData)
	{
		GetCharacterMovement()->MaxWalkSpeed = RowData->MovementSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = RowData->MovementSpeed;
		GetCharacterMovement()->MaxAcceleration = RowData->MaxAcceleration;
	}
}

void ANCBaseCharacter::ResetLandingState()
{
	CurrentActionTag = FGameplayTag::EmptyTag;
}
