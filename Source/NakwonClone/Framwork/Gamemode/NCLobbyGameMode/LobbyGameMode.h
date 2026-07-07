
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class NAKWONCLONE_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	ALobbyGameMode();

public:
	UFUNCTION(BlueprintCallable)
	void MoveToParkingLevel();
	
	UFUNCTION(BlueprintCallable)
	void MoveToTitle();
};
