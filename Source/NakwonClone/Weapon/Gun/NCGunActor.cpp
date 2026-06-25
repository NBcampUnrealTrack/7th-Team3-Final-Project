#include "NCGunActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCGunComponent.h"
#include "Weapon/Gun/GunType/NCGunType.h"

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

	// 같은 슬롯에 기존 총이 있으면 월드에 드롭
	const FName OldGunID = GunComp->GetOccupantGunID(GunID);
	if (!OldGunID.IsNone())
	{
		const FNCGunData* OldData = GunComp->GetGunData(OldGunID);
		if (OldData && OldData->GunActorClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			GetWorld()->SpawnActor<ANCGunActor>(OldData->GunActorClass, GetActorLocation(), GetActorRotation(), Params);
		}
	}

	if (GunComp->EquipGun(GunID))
		Destroy();
}
