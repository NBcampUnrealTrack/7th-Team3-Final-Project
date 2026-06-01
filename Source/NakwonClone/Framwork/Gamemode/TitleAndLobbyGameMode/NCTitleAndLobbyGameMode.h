
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NCTitleAndLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API ANCTitleAndLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ANCTitleAndLobbyGameMode();
	
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void MoveToGameLevel();
};
