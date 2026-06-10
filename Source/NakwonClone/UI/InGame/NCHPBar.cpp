#include "NCHPBar.h"

void UNCHPBar::UpdateHP(float CurrentHP, float MaxHP)
{
	if (!HPProgressBar || MaxHP <= 0.f)
	{
		return;
	}
	
	HPProgressBar->SetPercent(CurrentHP / MaxHP);
}