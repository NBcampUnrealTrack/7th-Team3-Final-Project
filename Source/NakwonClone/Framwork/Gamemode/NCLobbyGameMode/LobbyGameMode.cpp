
#include "LobbyGameMode.h"
#include "Engine/Engine.h"

ALobbyGameMode::ALobbyGameMode()
{
	DefaultPawnClass = nullptr;
}

// void ALobbyGameMode::MoveToParkingLevel()
// {
// 	GetWorld()->ServerTravel("/Game/Maps/L_Parking");	
// 	UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 실패로 서버트레블 실행"));
// }

void ALobbyGameMode::MoveToShopLevel()
{
	
	GetWorld()->ServerTravel("/Game/Maps/L_ShoppingMall");
	// if (GetWorld()->IsMapChangeReady())
	// {
	// 	GEngine->CommitMapChange(GetWorld());
	// }
	// else
	// {

	// }
}

void ALobbyGameMode::MoveToElderboomVillage()
{
	GetWorld()->ServerTravel("/Game/Maps/L_ElderboomVillage");
}

void ALobbyGameMode::MoveToTitle()
{
	GetWorld()->ServerTravel("/Game/Maps/L_TitleAndLobby");
}

