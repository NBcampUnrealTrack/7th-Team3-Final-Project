#include "NCEquipmentInventorySlot.h"
#include "GameplayTagContainer.h"
#include "Common/NCGameplayTags.h"
#include "Inventory/NCInventoryType.h"

void UNCEquipmentInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCEquipmentInventorySlot::SetSlotData(int32 InIndex, int32 InQuantity, FGameplayTag InTag, FName InItemID)
{
	if (bIsConsumableSlot && !InItemID.IsNone())
	{
		const bool bExpectHeal = (ConsumableIndex == 0);
		if (bExpectHeal  && !InTag.MatchesTag(NCItemTag::Heal)) return;
		if (!bExpectHeal && !InTag.MatchesTag(NCItemTag::Food)) return;
	}

	SlotIndex = InIndex;
	ItemTag = InTag;
	ItemID = InItemID;
	Quantity = InQuantity;
	
	if (!ItemID.IsNone())
	{
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));
		if (ItemData && ItemData->ItemIcon)
		{
			EquipmentItemImage->SetBrushFromTexture(ItemData->ItemIcon);
			
			if (ItemData->ItemTypeTag.MatchesTag(NCItemTag::Heal) || ItemData->ItemTypeTag.MatchesTag(NCItemTag::Food))
			{
				EquipmentItemQuantityTextBlock->SetText(FText::AsNumber(Quantity));
				EquipmentItemQuantityTextBlock->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				EquipmentItemQuantityTextBlock->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		
		EquipmentItemImage->SetVisibility(ESlateVisibility::Visible);
		EquipmentSlotImage->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		EquipmentSlotImage->SetVisibility(ESlateVisibility::Visible);
		EquipmentItemImage->SetVisibility(ESlateVisibility::Hidden);
		EquipmentItemQuantityTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}
