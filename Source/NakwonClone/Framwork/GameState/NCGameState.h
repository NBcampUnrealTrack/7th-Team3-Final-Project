// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Blueprint/UserWidget.h"

#include "NakwonClone/Common/NCGameplayTags.h"

#include "NCGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainingMatchTimeUpdate, int32, RemainingMatchTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChanged, int32, NewTotalScore, int32, GainedScore);

UCLASS()
class NAKWONCLONE_API ANCGameState : public AGameState
{
	GENERATED_BODY()
	
private:
	ANCGameState();
	
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
	
	UPROPERTY(ReplicatedUsing = OnRep_TotalScore, BlueprintReadOnly, Category = "Score")
	int32 TotalScore = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_bEscapable, BlueprintReadOnly, Category = "Score")
	bool bEscapable = false;
	
	UPROPERTY(BlueprintAssignable)
	FOnScoreChanged OnScoreChanged;

	UFUNCTION()
	void OnRep_bEscapable(); // 클라이언트에서 헬기 사운드 재생용

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHelicopterSound(USoundBase* Sound);
	
	UPROPERTY(EditDefaultsOnly, Category = "Escape")
	TSubclassOf<UUserWidget> ClearWidgetClass;
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_CurrentGameStateTag(); // UI 갱신용
	
	UFUNCTION()
	void OnRep_MatchTime();
	
	UFUNCTION(BlueprintCallable)
	int32 GetTotalScore() const {return TotalScore;}
	
	UFUNCTION()
    void OnRep_TotalScore(int32 OldTotalScore);
	
	UFUNCTION(BlueprintCallable)
	void AddWalkerKillCount() {  WalkerKillCount++ ;}
	
	UFUNCTION(BlueprintCallable)
	void AddRunnerKillCount() {  RunnerKillCount++ ;}
		
	UFUNCTION(BlueprintCallable)
	void AddWitchKillCount() {  WitchKillCount++ ;}
	
	UFUNCTION(BlueprintCallable)
	void AddTankKillCount() {  TankKillCount++ ;}
	
private:
		
	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 WalkerKillCount;
	
	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 RunnerKillCount;
		
	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 WitchKillCount;

	UPROPERTY(EditDefaultsOnly, Category = "Score")
	int32 TankKillCount;
};
