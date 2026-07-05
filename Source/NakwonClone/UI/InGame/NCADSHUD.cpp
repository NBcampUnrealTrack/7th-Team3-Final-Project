// 헌호수정 - 정조준(ADS) HUD 구현
#include "NCADSHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCEquipmentComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "NakwonClone/Weapon/Gun/GunType/NCGunType.h"

void UNCADSHUD::RefreshAll(ANCPlayerCharacter* Player)
{
	if (!Player)
	{
		return;
	}

	// ---------- HP ----------
	if (HPProgressBar)
	{
		if (UVGPlayerAttributeSet* Attr = Player->GetPlayerAttributeSet())
		{
			const float Max = Attr->GetMaxHealth();
			const float Cur = Attr->GetHealth();
			HPProgressBar->SetPercent(Max > 0.f ? FMath::Clamp(Cur / Max, 0.f, 1.f) : 0.f);
		}
	}

	UNCEquipmentComponent* Equip = Player->GetEquipmentComponent();

	// ---------- 탄약 ----------
	if (AmmoText)
	{
		if (Equip && Equip->HasActiveGun())
		{
			AmmoText->SetText(FText::FromString(
				FString::Printf(TEXT("%d / %d"), Equip->GetCurrentAmmo(), Equip->GetReserveAmmo())));
		}
		else
		{
			AmmoText->SetText(FText::FromString(TEXT("- / -")));
		}
	}

	// ---------- 무기 이름 ----------
	if (WeaponNameText)
	{
		FText WeaponName = FText::FromString(TEXT("-"));
		if (Equip)
		{
			if (const FNCGunData* GunData = Equip->GetActiveGunData())
			{
				WeaponName = GunData->DisplayName; // 총 이름 (Rifle / Pistol 등)
			}
			else if (!Player->StoredMeleeWeaponID.IsNone())
			{
				WeaponName = FText::FromString(TEXT("Melee")); // 근접무기
			}
		}
		WeaponNameText->SetText(WeaponName);
	}
}
