// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "NakwonClone/Common/NCGameplayTags.h"

#include "NCPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaBarChanged, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotImageChanged, UTexture2D*, ItemIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCitizenRankTextChanged, int32, CitizenRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTextChanged, int32, CharacterLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLifeStateChanged, FGameplayTag, LifeStateTag);

UCLASS()
class NAKWONCLONE_API ANCPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ANCPlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_LifeStateTag, BlueprintReadOnly)
	FGameplayTag LifeStateTag;
	
	UFUNCTION()
	void OnRep_LifeStateTag();
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadOnly)
	float CurrentHP;
	
	UFUNCTION()
	void OnRep_CurrentHP();
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float MaxHP;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStemina, BlueprintReadOnly)
	float CurrentStamina;
	
	UFUNCTION()
	void OnRep_CurrentStemina();
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float MaxStamina;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bHost;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 TeamIndex; // 멀티로 확장 시
	
public:
	UPROPERTY()
	FOnHPChanged OnHPChanged;
	
	UPROPERTY()
	FOnStaminaBarChanged OnStaminaBarChanged;
	
	UPROPERTY()
	FOnSlotImageChanged OnLeftSlotImageChanged;
	
	UPROPERTY()
	FOnSlotImageChanged OnRightSlotImageChanged;
	
	UPROPERTY()
	FOnCitizenRankTextChanged OnCitizenRankTextChanged;
	
	UPROPERTY()
	FOnLevelTextChanged OnLevelTextChanged;
	
	UPROPERTY()
	FOnLifeStateChanged OnLifeStateChanged;
};
