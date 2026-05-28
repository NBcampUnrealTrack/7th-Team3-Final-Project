#include "NCHPBar.h"

#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"

void UNCHPBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetOwningPlayerPawn());
	
	if (PlayerCharacter)
	{
		ANCPlayerState* PlayerState = Cast<ANCPlayerState>(PlayerCharacter->GetPlayerState());
		
		if (PlayerState)
		{
			PlayerState->OnHPChanged.AddDynamic(this, &UNCHPBar::UpdateHP);
		}
	}
}

void UNCHPBar::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPProgressBar)
	{
		HPProgressBar->SetPercent(CurrentHP / MaxHP);
	}
}