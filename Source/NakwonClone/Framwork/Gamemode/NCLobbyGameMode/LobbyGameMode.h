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
	
	FTimerHandle NextMapReadyTimerHandle;
	void TryCommitMapChange();
	
public:
	UFUNCTION(BlueprintCallable)
	void CheckMapReady();
	
	UFUNCTION(BlueprintCallable)
	void MoveToShopLevel();
};
