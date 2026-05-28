#include "NCLobbyLevelText.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"

void UNCLobbyLevelText::NativeConstruct()
{
	Super::NativeConstruct();
	
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetOwningPlayerPawn());
	
	if (PlayerCharacter)
	{
		PlayerCharacter->OnLevelTextChanged.AddDynamic(this, &UNCLobbyLevelText::UpdateLevelText);
	}
}

void UNCLobbyLevelText::UpdateLevelText(int32 CharacterLevel)
{
	if (LevelText)
	{
		LevelText->SetText(FText::AsNumber(CharacterLevel));
	}
}
