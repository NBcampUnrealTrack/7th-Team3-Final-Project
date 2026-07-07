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

	ItemCountText->SetVisibility(ESlateVisibility::Hidden);
	ItemImage->SetVisibility(ESlateVisibility::Hidden);

	if (ItemID.IsNone() || Quantity <= 0 || !ItemDataTable) return;

	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));
	if (!ItemData) return;

	if (ItemData->ItemTypeTag.MatchesTag(NCItemTag::Heal) || ItemData->ItemTypeTag.MatchesTag(NCItemTag::Food))
	{
		ItemCountText->SetText(FText::AsNumber(Quantity));
		ItemCountText->SetVisibility(ESlateVisibility::Visible);
	}
	UTexture2D* IconToShow = bIsLootBoxSlot && ItemData->LootBoxIcon ? ItemData->LootBoxIcon : ItemData->ItemIcon;
	if (IconToShow)
	{
		ItemImage->SetBrushFromTexture(IconToShow);
		ItemImage->SetVisibility(ESlateVisibility::Visible);
	}
}