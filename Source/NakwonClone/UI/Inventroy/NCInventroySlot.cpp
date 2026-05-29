#include "NCInventroySlot.h"

#include "GameplayTagContainer.h"

void UNCInventroySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UNCInventroySlot::SetSlotData(int32 InIndex, FGameplayTag InTag, int32 InQuantity)
{
	SlotIndex = InIndex;
	
	if (InTag.IsValid())
	{
		ItemCountText->SetText(FText::AsNumber(InQuantity));
		ItemImage->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ItemCountText->SetText(FText::GetEmpty());
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
	}
}
