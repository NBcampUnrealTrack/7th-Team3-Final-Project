#include "NCMeleePickupActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerAnimation/NCCombatComponent.h"
#include "Player/PlayerData/NCWeaponData.h"
#include "Player/PlayerComponent/NCGunComponent.h"

ANCMeleePickupActor::ANCMeleePickupActor()
{
}

void ANCMeleePickupActor::Interact_Implementation(AActor* Interactor)
{
	if (WeaponID.IsNone()) return;

	ANCPlayerCharacter* NCPlayer = Cast<ANCPlayerCharacter>(Interactor);
	if (!NCPlayer) return;

	UNCCombatComponent* NCCombat = NCPlayer->FindComponentByClass<UNCCombatComponent>();
	if (!NCCombat) return;

	UNCGunComponent* NCGunComp = NCPlayer->GetGunComponent();
	const bool bMeleeSlotActive = NCGunComp && !NCGunComp->HasActiveGun();
	const bool bWeaponInHand    = NCCombat->IsWeaponEquipped();

	// 기존 무기가 슬롯에 있으면 드랍 스폰 (손에 들고 있든 아니든)
	if (!NCPlayer->StoredMeleeWeaponID.IsNone())
	{
		if (NCPlayer->StoredMeleePickupClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ANCMeleePickupActor* Dropped = GetWorld()->SpawnActor<ANCMeleePickupActor>(
				NCPlayer->StoredMeleePickupClass,
				GetActorLocation(),
				NCPlayer->StoredMeleePickupRotation,
				Params);

			if (Dropped)
				Dropped->WeaponID = NCPlayer->StoredMeleeWeaponID;
		}

		// 손에 들고 있을 때만 해제
		if (bWeaponInHand)
			NCCombat->UnEquipWeapon();
	}

	// 새 무기 저장
	NCPlayer->StoredMeleeWeaponID      = WeaponID;
	NCPlayer->StoredMeleePickupClass   = GetClass();
	NCPlayer->StoredMeleePickupRotation = GetActorRotation();

	// 근접 슬롯이 활성 상태(3번)이고 손에 들려 있었다면 새 무기 바로 장착
	if (bMeleeSlotActive && bWeaponInHand)
	{
		FNCWeaponInstance Instance;
		Instance.WeaponID          = WeaponID;
		Instance.UniqueID          = FGuid::NewGuid();
		Instance.CurrentDurability = 100.f;
		NCCombat->EquipWeapon(Instance);
	}

	Destroy();
}
