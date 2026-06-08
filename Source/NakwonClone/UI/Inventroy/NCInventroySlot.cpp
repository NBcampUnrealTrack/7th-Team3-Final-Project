#include "NCInventroySlot.h"

#include "GameplayTagContainer.h"
#include "Inventory/NCInventoryType.h"

void UNCInventroySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCInventroySlot::SetSlotData(int32 InIndex, FGameplayTag InTag, int32 InQuantity, FName InItemID)
{
	SlotIndex = InIndex;
	ItemTag = InTag;
	Quantity = InQuantity;
	ItemID = InItemID;
	
	if (!ItemID.IsNone() && Quantity > 0)
	{
		ItemCountText->SetText(FText::AsNumber(InQuantity));
		ItemCountText->SetVisibility(ESlateVisibility::Visible);
		
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));
		if (ItemData && ItemData->ItemIcon)
		{
			ItemImage->SetBrushFromTexture(ItemData->ItemIcon);
		}
		
		ItemImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemCountText->SetText(FText::GetEmpty());
		ItemCountText->SetVisibility(ESlateVisibility::Hidden);
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
	}
}