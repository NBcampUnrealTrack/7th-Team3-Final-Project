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

	// 같은 슬롯에 기존 총이 있으면 탄약 상태 보존 후 월드에 드롭
	const FName OldGunID = EquipComp->GetOccupantGunID(GunID);
	if (!OldGunID.IsNone())
	{
		const FNCGunData* OldData = EquipComp->GetGunData(OldGunID);
		if (OldData && OldData->GunActorClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ANCGunActor* DroppedGun = GetWorld()->SpawnActor<ANCGunActor>(
				OldData->GunActorClass, GetActorLocation(), GetActorRotation(), Params);

			if (DroppedGun)
			{
				// 드롭 시점의 잔탄 저장
				const FNCGunSlotData& OldSlot = (OldData->SlotType == ENCGunSlot::Primary)
					? EquipComp->PrimarySlot : EquipComp->SecondarySlot;
				DroppedGun->SavedCurrentAmmo = OldSlot.CurrentAmmo;
				DroppedGun->SavedReserveAmmo = OldSlot.ReserveAmmo;
			}
		}
	}

	// 픽업 시 저장된 탄약이 있으면 복원, 없으면 풀 탄약으로 장착
	const bool bEquipped = (SavedCurrentAmmo >= 0)
		? EquipComp->EquipGunWithAmmo(GunID, SavedCurrentAmmo, SavedReserveAmmo)
		: EquipComp->EquipGun(GunID);

	if (bEquipped)
		Destroy();
}
