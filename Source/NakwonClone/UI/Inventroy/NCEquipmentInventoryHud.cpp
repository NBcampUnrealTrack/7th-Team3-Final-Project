#include "NCEquipmentInventoryHud.h"
#include "NCInventroySlot.h"
#include "NakwonClone/Inventory/NCInventoryType.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"

void UNCEquipmentInventoryHud::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCEquipmentInventoryHud::InitWithInventory(UNCInventoryBaseComponent* InInventory)
{
	if (!InInventory) return;
	
	InventoryComp = InInventory;
	
	InventoryComp->OnInventoryUpdated.AddDynamic(this, &UNCEquipmentInventoryHud::UpdateEquipmentItemSlot);
	
	UpdateEquipmentItemSlot();
}

void UNCEquipmentInventoryHud::UpdateEquipmentItemSlot()
{
	for (int32 i = 0; i < SlotWidgets.Num(); i++)
	{
		if (i < InventoryComp->EquipmentItem.Num())
		{
			const FInventorySlot& EquipmentSlot  = InventoryComp->EquipmentItem[i];

			SlotWidgets[i]->SetSlotData(i, EquipmentSlot.ItemTypeTag, EquipmentSlot.ItemID);
		}
		else
		{
			SlotWidgets[i]->SetSlotData(i, FGameplayTag(),NAME_None);
		}
	}
}
