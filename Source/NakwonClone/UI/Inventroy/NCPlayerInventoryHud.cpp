#include "NCPlayerInventoryHud.h"
#include "NCInventroySlot.h"
#include "NakwonClone/Inventory/NCInventoryType.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "NakwonClone/Player/PlayerController/NCPlayerController.h"

void UNCPlayerInventoryHud::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCPlayerInventoryHud::InitWithInventory(UNCInventoryBaseComponent* InInventory)
{
	if (!InInventory) return;
	
	InventoryComp = InInventory;
	
	int32 Columns = InventoryComp->GridSize.X;  // 열
	int32 Rows = InventoryComp->GridSize.Y;     // 행
	
	PlayerInventoryGrid->ClearChildren();
	SlotWidgets.Empty();
	
	for (int32 i = 0; i < Columns * Rows; i++)
	{
		UNCInventroySlot* ItemSlot = CreateWidget<UNCInventroySlot>(GetWorld(), SlotClass);
		PlayerInventoryGrid->AddChildToUniformGrid(ItemSlot, i / Columns, i % Columns);
		SlotWidgets.Add(ItemSlot);
	}
	
	InventoryComp->OnInventoryUpdated.AddDynamic(this, &UNCPlayerInventoryHud::UpdateItemSlot);

	UpdateItemSlot();
}

void UNCPlayerInventoryHud::UpdateItemSlot()
{
	for (int32 i = 0; i < SlotWidgets.Num(); i++)
	{
		if (i < InventoryComp->Items.Num())
		{
			const FInventorySlot& InventorySlot  = InventoryComp->Items[i];
			SlotWidgets[i]->SetSlotData(i, InventorySlot.ItemTypeTag, InventorySlot.Quantity, InventorySlot.ItemID);
		}
		else
		{
			SlotWidgets[i]->SetSlotData(i, FGameplayTag(), 0, NAME_None);
		}
	}
}
