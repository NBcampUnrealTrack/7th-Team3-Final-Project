
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
	
protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void MoveToGameLevel();
};
