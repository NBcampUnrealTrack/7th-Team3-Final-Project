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

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddPoints(int32 Points);

	void CheckPoints();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 SpecialZombieScore = 500;

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 EscapableScore = 100;

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	TObjectPtr<USoundBase> HelicopterSound;

	bool bSpecialZombieSpawned = false;
	
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void StartMatch() override;

	UFUNCTION(BlueprintCallable, Category = "GameFlow")
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
