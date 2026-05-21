#include "NCBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ANCBaseCharacter::ANCBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
}

void ANCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}
