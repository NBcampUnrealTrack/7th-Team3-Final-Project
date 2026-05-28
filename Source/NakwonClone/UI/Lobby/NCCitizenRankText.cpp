#include "NCCitizenRankText.h"

#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"

void UNCCitizenRankText::NativeConstruct()
{
	Super::NativeConstruct();
	
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetOwningPlayerPawn());
	
	if (PlayerCharacter)
	{
		ANCPlayerState* PlayerState = Cast<ANCPlayerState>(PlayerCharacter->GetPlayerState());
		
		if (PlayerState)
		{
			PlayerState->OnCitizenRankTextChanged.AddDynamic(this, &UNCCitizenRankText::UpdateCitizenRankText);
		}
	}
}

void UNCCitizenRankText::UpdateCitizenRankText(int32 CitizenRank)
{
	if (CitizenRankText)
	{
		CitizenRankText->SetText(FText::AsNumber(CitizenRank));
	}
}
