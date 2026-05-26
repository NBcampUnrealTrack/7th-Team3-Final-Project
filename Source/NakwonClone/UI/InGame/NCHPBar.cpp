#include "NCHPBar.h"

void UNCHPBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 캐릭터 델리게이트 연결
}

void UNCHPBar::OnHPChanged(float CurrentHP, float MaxHP)
{
	if (HPProgressBar)
	{
		HPProgressBar->SetPercent(CurrentHP / MaxHP);
	}
}