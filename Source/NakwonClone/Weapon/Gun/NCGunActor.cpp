#include "NCGunActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"
#include "Weapon/Gun/GunType/NCGunType.h"

ANCGunActor::ANCGunActor()
{
}

void ANCGunActor::Interact_Implementation(AActor* Interactor)
{
	if (GunID.IsNone()) return;

	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	UNCEquipmentComponent* EquipComp = Player->GetEquipmentComponent();
	if (!EquipComp) return;

	// 픽업 시 저장된 탄약이 있으면 복원, 없으면 풀 탄약으로 장착
	const bool bEquipped = (SavedCurrentAmmo >= 0)
		? EquipComp->EquipGunWithAmmo(GunID, SavedCurrentAmmo, SavedReserveAmmo)
		: EquipComp->EquipGun(GunID);

	if (bEquipped)
		Destroy();
}
