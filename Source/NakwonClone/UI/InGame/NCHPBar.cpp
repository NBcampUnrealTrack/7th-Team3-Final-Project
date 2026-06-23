#include "NCHPBar.h"
#include "Components/HorizontalBoxSlot.h"

void UNCHPBar::UpdateHP(float CurrentHP, float MaxHP)
{
	if (!HPProgressBar || MaxHP <= 0.f)
	{
		return;
	}
	
	const float Percent = FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f);
	
	HPProgressBar->SetPercent(Percent);
	
	if (LeftSpacer)
	{
		if (UHorizontalBoxSlot* LeftSlot = Cast<UHorizontalBoxSlot>(LeftSpacer->Slot))
		{
			FSlateChildSize LeftSize(ESlateSizeRule::Fill);
			LeftSize.Value = Percent;
			LeftSlot->SetSize(LeftSize);
		}
	}

	if (RightSpacer)
	{
		if (UHorizontalBoxSlot* RightSlot = Cast<UHorizontalBoxSlot>(RightSpacer->Slot))
		{
			FSlateChildSize RightSize(ESlateSizeRule::Fill);
			RightSize.Value = 1.f - Percent;
			RightSlot->SetSize(RightSize);
		}
	}

	if (HPText)
	{
		HPText->SetText(FText::AsNumber(FMath::RoundToInt(CurrentHP)));
	}
}