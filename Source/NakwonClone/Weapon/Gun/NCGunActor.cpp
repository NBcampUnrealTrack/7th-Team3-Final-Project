#include "NCGunActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCGunComponent.h"

ANCGunActor::ANCGunActor()
{
}

void ANCGunActor::Interact_Implementation(AActor* Interactor)
{
	if (GunID.IsNone()) return;

	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	UNCGunComponent* GunComp = Player->FindComponentByClass<UNCGunComponent>();
	if (!GunComp) return;

	if (GunComp->EquipGun(GunID))
		Destroy();
}
