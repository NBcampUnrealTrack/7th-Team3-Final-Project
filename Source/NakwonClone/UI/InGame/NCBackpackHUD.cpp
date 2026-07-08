// 헌호수정 - 백팩(등) 3D HUD 구현
#include "NCBackpackHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCGunComponent.h"
#include "NakwonClone/Player/PlayerComponent/NCEquipmentComponent.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "NakwonClone/Weapon/Gun/GunType/NCGunType.h"
#include "NakwonClone/Inventory/NCInventoryType.h"

void UNCBackpackHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwningPlayerPawn()))
	{
		RefreshAll(Player);
	}
}

void UNCBackpackHUD::RefreshAll(ANCPlayerCharacter* Player)
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

	// ---------- 탄약 (현재 장착한 총) ----------
	if (AmmoText)
	{
		UNCGunComponent* Gun = Player->GetGunComponent();
		if (Gun && Gun->HasActiveGun())
		{
			AmmoText->SetText(FText::FromString(
				FString::Printf(TEXT("%d / %d"), Gun->CurrentAmmo, Gun->ReserveAmmo)));
		}
		else
		{
			AmmoText->SetText(FText::FromString(TEXT("- / -"))); // 총 없음(근접/맨손)
		}
	}

	// ---------- 소지품 (힐 / 스낵) ----------
	if (UNCPlayerInventoryComponent* Inv = Player->GetInventoryComponent())
	{
		if (HealText)
		{
			const FInventorySlot Heal = Inv->GetConsumableData(0, 0); // 0 = 힐
			HealText->SetText(FText::AsNumber(Heal.Quantity));
		}
		if (SnackText)
		{
			const FInventorySlot Snack = Inv->GetConsumableData(0, 1); // 1 = 스낵/음식
			SnackText->SetText(FText::AsNumber(Snack.Quantity));
		}
	}

	// ---------- 무기 슬롯 1/2/3 색상 (활성=초록, 장착=흰색, 빈칸=회색) ----------
	const FLinearColor ColActive(0.2f, 1.f, 0.2f, 1.f);   // 밝은 초록 (지금 든 무기)
	const FLinearColor ColEquipped(1.f, 1.f, 1.f, 1.f);   // 흰색 (무기 있지만 안 든 것)
	const FLinearColor ColEmpty(0.35f, 0.35f, 0.35f, 1.f); // 어두운 회색 (빈 슬롯)

	if (UNCEquipmentComponent* Equip = Player->GetEquipmentComponent())
	{
		const ENCGunSlot Active = Equip->ActiveSlot;

		// 슬롯 1 (Primary)
		if (Weapon1Text)
		{
			const bool bEquipped = !Equip->PrimarySlot.GunID.IsNone();
			const bool bActive = (Active == ENCGunSlot::Primary);
			Weapon1Text->SetColorAndOpacity(FSlateColor(
				!bEquipped ? ColEmpty : (bActive ? ColActive : ColEquipped)));
		}

		// 슬롯 2 (Secondary)
		if (Weapon2Text)
		{
			const bool bEquipped = !Equip->SecondarySlot.GunID.IsNone();
			const bool bActive = (Active == ENCGunSlot::Secondary);
			Weapon2Text->SetColorAndOpacity(FSlateColor(
				!bEquipped ? ColEmpty : (bActive ? ColActive : ColEquipped)));
		}
	}

	// 슬롯 3 (근접무기) — 활성 조건: 총이 없을 때(None)
	if (Weapon3Text)
	{
		const bool bEquipped = !Player->StoredMeleeWeaponID.IsNone();
		bool bActive = false;
		if (UNCEquipmentComponent* Equip = Player->GetEquipmentComponent())
		{
			bActive = (Equip->ActiveSlot == ENCGunSlot::None);
		}
		Weapon3Text->SetColorAndOpacity(FSlateColor(
			!bEquipped ? ColEmpty : (bActive ? ColActive : ColEquipped)));
	}
}
