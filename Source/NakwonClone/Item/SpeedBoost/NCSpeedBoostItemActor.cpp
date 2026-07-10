#include "NCSpeedBoostItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/Locomotion/UNCLocomotionComponent.h"

void ANCSpeedBoostItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	UNCLocomotionComponent* LocomotionComp = Player->GetLocomotionComponent();
	if (!LocomotionComp) return;

	LocomotionComp->ActivateSpeedBoost(SpeedMultiplier, Duration);

	ConsumeItem();
}