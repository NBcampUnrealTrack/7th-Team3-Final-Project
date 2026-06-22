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
		InventoryComp->OnPresetUpdated.AddDynamic(this, &UNCItemSlot::UpdateSlotVisual);

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
	
	int32 PresetIndex = InventoryComp->CurrentEquippedPresetIndex;
	if (PresetIndex < 0)
	{
		PresetIndex = 0;
	}

	FEquipmentPreset Preset = InventoryComp->GetPresetData(PresetIndex);

	FInventorySlot LeftSlotData  = Preset.IsTwoHandActive() ? Preset.TwoHand : Preset.LeftHand;
	FInventorySlot RightSlotData = Preset.IsTwoHandActive() ? Preset.TwoHand : Preset.RightHand;

	UpdateImage(LeftSlotImage, LeftSlotData);
	UpdateImage(RightSlotImage, RightSlotData);

	const int32 RawPresetIndex = InventoryComp->CurrentEquippedPresetIndex; // -1 = 맨손, 0 = 프리셋1, 1 = 프리셋2
	const FLinearColor HighlightColor(0.7f, 0.7f, 0.7f, 1.f);
	const FLinearColor DimColor(0.3f, 0.3f, 0.3f, 0.5f);
	
	if (RawPresetIndex == 0)
	{
		FirstItemSlotImage->SetColorAndOpacity(HighlightColor);
		FirstItemSlotNumber->SetColorAndOpacity(FSlateColor(HighlightColor));
		SecondItemSlotImage->SetColorAndOpacity(DimColor);
		SecondItemSlotNumber->SetColorAndOpacity(FSlateColor(DimColor));
	}
	if (RawPresetIndex == 1)
	{
		FirstItemSlotImage->SetColorAndOpacity(DimColor);
		FirstItemSlotNumber->SetColorAndOpacity(FSlateColor(DimColor));
		SecondItemSlotImage->SetColorAndOpacity(HighlightColor);
		SecondItemSlotNumber->SetColorAndOpacity(FSlateColor(HighlightColor));
	}
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