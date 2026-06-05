#include "NCInventroySlot.h"

#include "GameplayTagContainer.h"

void UNCInventroySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCInventroySlot::SetSlotData(int32 InIndex, FGameplayTag InTag, int32 InQuantity)
{
	SlotIndex = InIndex;
	ItemTag = InTag;
	Quantity = InQuantity;
	
	if (InTag.IsValid())
	{
		ItemCountText->SetText(FText::AsNumber(InQuantity));
		ItemCountText->SetVisibility(ESlateVisibility::Visible);
		ItemImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemCountText->SetText(FText::GetEmpty());
		ItemCountText->SetVisibility(ESlateVisibility::Hidden);
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
	}
}