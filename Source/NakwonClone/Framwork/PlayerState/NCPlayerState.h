// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "NakwonClone/Common/NCGameplayTags.h"

#include "NCPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API ANCPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_LifeStateTag, BlueprintReadOnly)
	FGameplayTag LifeStateTag;
	
	UFUNCTION()
	void OnRep_LifeStateTag();
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float CurrentHP;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float MaxHP;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bHost;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 TeamIndex; // 멀티로 확장 시
	
};
