
#include "LobbyGameMode.h"
#include "Engine/Engine.h"

ALobbyGameMode::ALobbyGameMode()
{
	DefaultPawnClass = nullptr;
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::MoveToParkingLevel()
{
	GetWorld()->ServerTravel("/Game/Maps/L_Parking");	
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 실패로 서버트레블 실행"));
}
