#include "NCEquipmentInventorySlot.h"
#include "GameplayTagContainer.h"
#include "Inventory/NCInventoryType.h"

void UNCEquipmentInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCEquipmentInventorySlot::SetSlotData(int32 InIndex, FGameplayTag InTag, FName InItemID)
{
	UE_LOG(LogTemp, Warning, TEXT("ItemImage %s"),
	EquipmentItemImage ? TEXT("VALID") : TEXT("NULL"));
	
	SlotIndex = InIndex;
	ItemTag = InTag;
	ItemID = InItemID;
	
	if (!ItemID.IsNone())
	{
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT(""));
		if (ItemData && ItemData->ItemIcon)
		{
			EquipmentItemImage->SetBrushFromTexture(ItemData->ItemIcon);
		}
		
		EquipmentItemImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		EquipmentItemImage->SetVisibility(ESlateVisibility::Hidden);
	}
}
