#include "NCMeleePickupActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerAnimation/NCCombatComponent.h"
#include "Player/PlayerData/NCWeaponData.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"

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

	UNCEquipmentComponent* NCGunComp = NCPlayer->GetEquipmentComponent();
	const bool bMeleeSlotActive = NCGunComp && !NCGunComp->HasActiveGun();
	const bool bWeaponInHand    = NCCombat->IsWeaponEquipped();

	if (bWeaponInHand)
	{
		NCCombat->UnEquipWeapon();
	}

	// 새 무기 저장
	NCPlayer->StoredMeleeWeaponID      = WeaponID;
	NCPlayer->StoredMeleePickupClass   = GetClass();
	NCPlayer->StoredMeleePickupRotation = GetActorRotation();

	// 핫바 UI 갱신용: 보관 무기가 바뀌었음을 알림 (줍는 즉시 3번 칸 아이콘 갱신)
	NCPlayer->OnMeleeStoredChanged.Broadcast();

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
