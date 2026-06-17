// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"

#include "NakwonClone/Common/NCGameplayTags.h"

#include "NCGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingMatchTimeUpdate, int32, RemainingMatchTime);

UCLASS()
class NAKWONCLONE_API ANCGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 ConnectedPlayerCount;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 AlivePlayerCount;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGameStateTag, BlueprintReadOnly, Category="GameFlow")
	FGameplayTag CurrentGameStateTag;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchTime, BlueprintReadOnly)
	int32 RemainingMatchTime;
	
	UPROPERTY()
	FOnRemainingMatchTimeUpdate OnRemainingMatchTimeUpdate;
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// UFUNCTION()
	// float GetRemainingMatchTime();
	
	UFUNCTION()
	void OnRep_CurrentGameStateTag(); // UI 갱신용
	
	UFUNCTION()
	void OnRep_MatchTime();
};
