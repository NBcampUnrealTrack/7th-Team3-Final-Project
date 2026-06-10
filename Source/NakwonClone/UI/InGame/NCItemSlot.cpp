#include "NCItemSlot.h"

#include "Inventory/NCInventoryType.h"
#include "Kismet/GameplayStatics.h"
#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "Player/PlayerComponent/NCPlayerInventoryComponent.h"

void UNCItemSlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerState) return;

	UNCPlayerInventoryComponent* InventoryComp = PC->PlayerState->FindComponentByClass<UNCPlayerInventoryComponent>();
	if (InventoryComp)
	{
		InventoryComp->OnQuickSlotUpdated.RemoveDynamic(this, &UNCItemSlot::UpdateSlotVisual);
		InventoryComp->OnQuickSlotUpdated.AddDynamic(this, &UNCItemSlot::UpdateSlotVisual);

		UpdateSlotVisual();
	}
}

void UNCItemSlot::UpdateSlotVisual()
{
	if (!RightSlotImage) return;
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerState) return;

	UNCPlayerInventoryComponent* InventoryComp = PC->PlayerState->FindComponentByClass<UNCPlayerInventoryComponent>();
	if (!InventoryComp) return;
	
	FInventorySlot SlotData = InventoryComp->GetQuickSlotData(TargetSlotIndex);
	
	static const FString ContextString(TEXT("QuickSlot Update Context"));
	FItemData* RowData = ItemDataTable->FindRow<FItemData>(SlotData.ItemID, ContextString);

	if (RowData && RowData->ItemIcon)
	{
		// 4. 데이터 테이블에 등록된 아이템 실제 아이콘으로 교체
		RightSlotImage->SetBrushFromTexture(RowData->ItemIcon);
	}
}