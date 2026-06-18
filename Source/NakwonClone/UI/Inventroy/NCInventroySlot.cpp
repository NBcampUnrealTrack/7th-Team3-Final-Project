#include "NCInventroySlot.h"

#include "GameplayTagContainer.h"
#include "Common/NCGameplayTags.h"
#include "Inventory/NCInventoryType.h"

void UNCInventroySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCInventroySlot::SetSlotData(int32 InIndex, int32 InQuantity, FGameplayTag InTag, FName InItemID)
{
	SlotIndex = InIndex;
	ItemTag = InTag;
	Quantity = InQuantity;
	ItemID = InItemID;
	
	if (!ItemID.IsNone() && Quantity > 0)
	{
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));
		if (!ItemData)
		{
			ItemCountText->SetVisibility(ESlateVisibility::Hidden);
			ItemImage->SetVisibility(ESlateVisibility::Hidden);
		}
		if (ItemData->ItemTypeTag.MatchesTag(NCItemTag::Heal) || ItemData->ItemTypeTag.MatchesTag(NCItemTag::Food))
		{
			ItemCountText->SetText(FText::AsNumber(Quantity));
			ItemCountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemCountText->SetVisibility(ESlateVisibility::Hidden);
		}
		
		if (ItemData && ItemData->ItemIcon)
		{
			ItemImage->SetBrushFromTexture(ItemData->ItemIcon);
		}
		
		ItemImage->SetVisibility(ESlateVisibility::Visible);
	}
}