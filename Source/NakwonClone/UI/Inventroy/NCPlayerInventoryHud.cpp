#include "NCPlayerInventoryHud.h"
#include "NCInventroySlot.h"
#include "NakwonClone/Inventory/NCInventoryType.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogInvHud, Log, All);

// 크래시가 나도 마지막 줄이 로그 파일(Saved/Logs)에 남도록 즉시 flush
#define INVHUD_TRACE(Fmt, ...) do { UE_LOG(LogInvHud, Warning, TEXT(Fmt), ##__VA_ARGS__); if (GLog) { GLog->Flush(); } } while(0)

void UNCPlayerInventoryHud::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCPlayerInventoryHud::InitWithInventory(UNCInventoryBaseComponent* InInventory)
{
	INVHUD_TRACE("[1] InitWithInventory enter. this=%p InInventory=%p", this, InInventory);

	if (!IsValid(InInventory)) { INVHUD_TRACE("[X] InInventory invalid -> return"); return; }   // null + dangling 차단
	if (!PlayerInventoryGrid) { INVHUD_TRACE("[X] PlayerInventoryGrid is NULL -> return"); return; } // BindWidget 실패
	if (!SlotClass)           { INVHUD_TRACE("[X] SlotClass is NULL -> return"); return; }            // SlotClass 미지정

	InventoryComp = InInventory;

	const int32 Columns = InventoryComp->GridSize.X;  // 열
	const int32 Rows = InventoryComp->GridSize.Y;     // 행
	INVHUD_TRACE("[2] GridSize Columns=%d Rows=%d", Columns, Rows);
	if (Columns <= 0 || Rows <= 0) { INVHUD_TRACE("[X] bad GridSize -> return"); return; }            // 쓰레기 GridSize 차단

	PlayerInventoryGrid->ClearChildren();
	SlotWidgets.Empty();

	INVHUD_TRACE("[3] creating %d slots", Columns * Rows);
	for (int32 i = 0; i < Columns * Rows; i++)
	{
		UNCInventroySlot* ItemSlot = CreateWidget<UNCInventroySlot>(GetWorld(), SlotClass);
		if (!ItemSlot) continue;
		PlayerInventoryGrid->AddChildToUniformGrid(ItemSlot, i / Columns, i % Columns);
		SlotWidgets.Add(ItemSlot);
	}

	INVHUD_TRACE("[4a] slots created=%d. before AddDynamic", SlotWidgets.Num());
	// AddDynamic은 중복 제거를 안 함 -> 재호출 시 다중 바인딩 방지를 위해 Remove 후 Add
	InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UNCPlayerInventoryHud::UpdateItemSlot);
	InventoryComp->OnInventoryUpdated.AddDynamic(this, &UNCPlayerInventoryHud::UpdateItemSlot);

	INVHUD_TRACE("[4b] bound. before UpdateItemSlot");
	UpdateItemSlot();
	INVHUD_TRACE("[5] InitWithInventory done");
}

void UNCPlayerInventoryHud::UpdateItemSlot()
{
	INVHUD_TRACE("[U0] UpdateItemSlot enter. this=%p Slots=%d InvValid=%d", this, SlotWidgets.Num(), IsValid(InventoryComp) ? 1 : 0);
	if (!IsValid(InventoryComp)) { INVHUD_TRACE("[UX] InventoryComp invalid -> return"); return; }

	const int32 ItemNum = InventoryComp->Items.Num();
	for (int32 i = 0; i < SlotWidgets.Num(); i++)
	{
		if (!SlotWidgets[i]) { INVHUD_TRACE("[UX] SlotWidgets[%d] is NULL -> skip", i); continue; }

		SlotWidgets[i]->OwningInventory = InventoryComp;

		if (i < ItemNum)
		{
			const FInventorySlot& InventorySlot  = InventoryComp->Items[i];
			SlotWidgets[i]->SetSlotData(i, InventorySlot.Quantity, InventorySlot.ItemTypeTag, InventorySlot.ItemID);
		}
		else
		{
			SlotWidgets[i]->SetSlotData(i, 0, FGameplayTag() ,NAME_None);
		}
	}
	INVHUD_TRACE("[U1] UpdateItemSlot done");
}
