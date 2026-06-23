// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	ALobbyGameMode();
	
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable)
	void MoveToShopLevel();
	
};
