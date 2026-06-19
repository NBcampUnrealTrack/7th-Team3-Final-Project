#include "NCLootBoxHud.h"
#include "Components/UniformGridPanel.h"
#include "UI/Inventroy/NCInventroySlot.h"
#include "NakwonClone/Item/ANCLootBoxActor.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"
#include "Player/PlayerController/NCPlayerController.h"

void UNCLootBoxHud::InitWithLootBox(AANCLootBoxActor* TargetBox, UNCInventoryBaseComponent* PlayerInventory)
{
	if (!TargetBox || !LootGrid || !SlotClass) return;
	
	CurrentLootBox = TargetBox;
	LootInventoryComp = TargetBox->GetLootInventory();

	if (!LootInventoryComp) return;

	// 인벤토리 실제 크기에 맞게 슬롯 생성
	int32 TotalSlots = LootInventoryComp->Items.Num();
	int32 Columns = LootInventoryComp->GridSize.X;

	LootGrid->ClearChildren();
	LootSlotWidgets.Empty();

	for (int32 i = 0; i < TotalSlots; i++)
	{
		UNCInventroySlot* NewSlot = CreateWidget<UNCInventroySlot>(GetOwningPlayer(), SlotClass);
		NewSlot->bIsLootBoxSlot = true;
		NewSlot->LootBoxRef = CurrentLootBox;
		LootGrid->AddChildToUniformGrid(NewSlot, i / Columns, i % Columns);
		LootSlotWidgets.Add(NewSlot);
	}

	LootInventoryComp->OnInventoryUpdated.AddDynamic(this, &UNCLootBoxHud::UpdateLootSlots);
	UpdateLootSlots();
}

void UNCLootBoxHud::CloseLootBoxUI()
{
	if (CurrentLootBox)
	{
		CurrentLootBox->EndLooting();
	}
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ANCPlayerController* NCPC = Cast<ANCPlayerController>(PC))
		{
			if (NCPC->bIsInventoryOpen)
			{
				NCPC->ToggleInventory();
			}
		}
	}

	
	RemoveFromParent();
}

void UNCLootBoxHud::NativeConstruct()
{
	Super::NativeConstruct();
	
	// LootGrid는 디자이너에서 배치한 UniformGridPanel과 자동 연결됨
	// 슬롯은 InitWithLootBox 호출 시 실제 인벤토리 크기에 맞게 생성
}

void UNCLootBoxHud::UpdateLootSlots()
{
	for (int32 i = 0; i < LootSlotWidgets.Num(); i++)
	{
		const FInventorySlot& ItemSlot  = LootInventoryComp->Items[i];
		LootSlotWidgets[i]->SetSlotData(i, ItemSlot.Quantity, ItemSlot.ItemTypeTag, ItemSlot.ItemID);
	}
}
