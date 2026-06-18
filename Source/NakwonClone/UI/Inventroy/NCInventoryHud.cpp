#include "NCInventoryHud.h"
#include "NCInventroySlot.h"
#include "NakwonClone/Inventory/NCInventoryType.h"

void UNCInventoryHud::NativeConstruct()
{
	Super::NativeConstruct();
	
	int32 Columns = 6;  // 열
	int32 Rows = 5;     // 행
	
	for (int32 i = 0; i < Columns * Rows; i++)
	{
		UNCInventroySlot* ItemSlot = CreateWidget<UNCInventroySlot>(GetWorld(), SlotClass);
		
		InventoryGrid->AddChildToUniformGrid(ItemSlot, i / Columns, i % Columns);
		
		SlotWidgets.Add(ItemSlot);
	}
}

void UNCInventoryHud::InitWithInventory(UNCInventoryBaseComponent* InInventory)
{
	if (!InInventory) return;
	
	InventoryComp = InInventory;

	InventoryComp->OnInventoryUpdated.AddDynamic(
		this, &UNCInventoryHud::UpdateItemSlot);

	UpdateItemSlot();
}

void UNCInventoryHud::UpdateItemSlot()
{
	for (int32 i = 0; i < SlotWidgets.Num(); i++)
	{
		const FInventorySlot& InventorySlot  = InventoryComp->Items[i];
		SlotWidgets[i]->SetSlotData(i, InventorySlot.Quantity, InventorySlot.ItemTypeTag, InventorySlot.ItemID);
	}
}