#include "NCLobbyLevelText.h"

void UNCLobbyLevelText::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 캐릭터 델리게이트 연결
}

void UNCLobbyLevelText::OnTimerTextChanged()
{
	if (LevelText)
	{
		//LevelText->SetText();
	}
}
