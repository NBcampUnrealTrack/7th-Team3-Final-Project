
#include "LobbyGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
	DefaultPawnClass = nullptr;
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::MoveToParkingLevel()
{
	GetWorld()->ServerTravel("/Game/Maps/L_Parking");
}
