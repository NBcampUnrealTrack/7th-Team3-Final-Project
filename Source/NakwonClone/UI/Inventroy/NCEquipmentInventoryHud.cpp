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
	
	int32 Columns = 2;  // 열
	int32 Rows = 3;     // 행
	
	PlayerInventoryGrid->ClearChildren();
	SlotWidgets.Empty();
	
	for (int32 i = 0; i < Columns * Rows; i++)
	{
		UNCEquipmentInventorySlot* ItemSlot = CreateWidget<UNCEquipmentInventorySlot>(GetWorld(), SlotClass);
		PlayerInventoryGrid->AddChildToUniformGrid(ItemSlot, i / Columns, i % Columns);
		SlotWidgets.Add(ItemSlot);
		
		UE_LOG(LogTemp, Warning,TEXT("Create Slot %d : %s"),i,ItemSlot ? TEXT("VALID") : TEXT("NULL"));
	}

	
	InventoryComp->OnInventoryUpdated.AddDynamic(this, &UNCEquipmentInventoryHud::UpdateEquipmentItemSlot);

	UE_LOG(LogTemp, Warning,
    TEXT("Grid Children = %d"),
    PlayerInventoryGrid->GetChildrenCount());
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
