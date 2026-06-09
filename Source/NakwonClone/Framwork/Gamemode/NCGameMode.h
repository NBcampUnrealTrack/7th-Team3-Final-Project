// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NakwonClone/Framwork/GameState/NCGameState.h"
#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "NakwonClone/Common/NCGameplayTags.h"

#include "NCGameMode.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API ANCGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ANCGameMode();
	
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void StartMatch() override;
	void HandleMatchEnd(bool bClear);
	
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void MoveToTitle();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void MoveToLobby();

	void HandlePlayerDowned(ANCPlayerState* PlayerState);
	void HandlePlayerRevived(ANCPlayerState* PlayerState);
	void HandlePlayerDead(ANCPlayerState* PlayerState);
	
	void CheckAllPlayersDead();
	
	FTimerHandle MatchTimerHandle;
	
	UFUNCTION(BlueprintCallable)
	void SetMatchTimerHandle();
	
	void TimerTick();
};
