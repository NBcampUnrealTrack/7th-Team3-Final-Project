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
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerState) return;

	UNCPlayerInventoryComponent* InventoryComp =
		PC->PlayerState->FindComponentByClass<UNCPlayerInventoryComponent>();

	if (!InventoryComp) return;
	
	FEquipmentPreset Preset0 = InventoryComp->GetPresetData(0);
	FInventorySlot LeftSlotData = Preset0.IsTwoHandActive() ? Preset0.TwoHand : Preset0.RightHand;

	FEquipmentPreset Preset1 = InventoryComp->GetPresetData(1);
	FInventorySlot RightSlotData = Preset1.IsTwoHandActive() ? Preset1.TwoHand : Preset1.RightHand;

	UpdateImage(LeftSlotImage, LeftSlotData);
	UpdateImage(RightSlotImage, RightSlotData);
}

void UNCItemSlot::UpdateImage(UImage* TargetImage, const FInventorySlot& SlotData)
{
	if (!TargetImage) return;
	
	if (SlotData.ItemID.IsNone())
	{
		if (TargetImage == LeftSlotImage)
		{
			TargetImage->SetBrushFromTexture(LeftSlotDefaultImage);
		}
		else if (TargetImage == RightSlotImage)
		{
			TargetImage->SetBrushFromTexture(RightSlotDefaultImage);
		}

		return;
	}

	static const FString ContextString(TEXT("QuickSlot"));

	FItemData* RowData = ItemDataTable->FindRow<FItemData>(SlotData.ItemID, ContextString);

	if (RowData && RowData->ItemIcon)
	{
		TargetImage->SetBrushFromTexture(RowData->ItemIcon);
	}
}