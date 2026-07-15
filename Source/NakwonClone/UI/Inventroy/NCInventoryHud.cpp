#include "NCInventoryHud.h"
#include "NCInventroySlot.h"
#include "Player/PlayerComponent/NCGunComponent.h"

void UNCInventoryHud::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwningPlayerPawn()))
	{
		AmmoUpdate(Player);
	}
}

// void UNCInventoryHud::CreateSlots()
// {
// 	// 이미 만들어져 있으면 중복 생성 방지 (NativeConstruct / InitWithInventory 중 먼저 호출된 쪽이 생성)
// 	if (SlotWidgets.Num() > 0) return;
//
// 	int32 Columns = 6;  // 열
// 	int32 Rows = 5;     // 행
//
// 	for (int32 i = 0; i < Columns * Rows; i++)
// 	{
// 		UNCInventroySlot* ItemSlot = CreateWidget<UNCInventroySlot>(GetWorld(), SlotClass);
//
// 		InventoryGrid->AddChildToUniformGrid(ItemSlot, i / Columns, i % Columns);
//
// 		SlotWidgets.Add(ItemSlot);
// 	}
// }
//
// void UNCInventoryHud::InitWithInventory(UNCInventoryBaseComponent* InInventory)
// {
// 	if (!InInventory) return;
//
// 	InventoryComp = InInventory;
//
// 	CreateSlots();
//
// 	InventoryComp->OnInventoryUpdated.AddDynamic(
// 		this, &UNCInventoryHud::UpdateItemSlot);
//
// 	UpdateItemSlot();
// }
//
// void UNCInventoryHud::UpdateItemSlot()
// {
// 	for (int32 i = 0; i < SlotWidgets.Num(); i++)
// 	{
// 		SlotWidgets[i]->OwningInventory = InventoryComp;
//
// 		if (i < InventoryComp->Items.Num())
// 		{
// 			const FInventorySlot& InventorySlot  = InventoryComp->Items[i];
// 			SlotWidgets[i]->SetSlotData(i, InventorySlot.Quantity, InventorySlot.ItemTypeTag, InventorySlot.ItemID);
// 		}
// 		else
// 		{
// 			SlotWidgets[i]->SetSlotData(i, 0, FGameplayTag(), NAME_None);
// 		}
// 	}
// }


void UNCInventoryHud::AmmoUpdate(ANCPlayerCharacter* Player)
{
	if (AmmoText)
	{
		UNCGunComponent* Gun = Player->GetGunComponent();
		if (Gun && Gun->HasActiveGun())
		{
			AmmoText->SetText(FText::FromString(
				FString::Printf(TEXT("%d / %d"), Gun->CurrentAmmo, Gun->ReserveAmmo)));
			AmmoText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			AmmoText->SetText(FText::FromString(TEXT("- / -"))); // 총 없음(근접/맨손)
			AmmoText->SetVisibility(ESlateVisibility::Hidden);
		}
	}	
}