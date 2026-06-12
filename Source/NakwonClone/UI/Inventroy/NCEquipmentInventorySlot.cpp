#include "NCEquipmentInventorySlot.h"

#include <strmif.h>

#include "GameplayTagContainer.h"
#include "Common/NCGameplayTags.h"
#include "Inventory/NCInventoryType.h"

void UNCEquipmentInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCEquipmentInventorySlot::SetSlotData(int32 InIndex, int32 InQuantity, FGameplayTag InTag, FName InItemID)
{
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
	}
	else
	{
		EquipmentItemImage->SetVisibility(ESlateVisibility::Hidden);
		EquipmentItemQuantityTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}
