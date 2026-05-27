// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "NakwonClone/Common/NCGameplayTags.h"

#include "NCGameState.generated.h"

UCLASS()
class NAKWONCLONE_API ANCGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // todo : 내용 확인
	
	UPROPERTY(ReplicatedUsing = OnRep_GameStateTag, BlueprintReadOnly)
	FGameplayTag CurrentGameStateTag;
	UFUNCTION()
	void OnRep_GameStateTag(); // UI 갱신용
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchTime, BlueprintReadOnly)
	float RemainingMatchTime;
	UFUNCTION()
	void OnRep_MatchTime();
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 ConnectedPlayerCount;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 AlivePlayerCount;
	
	
};
