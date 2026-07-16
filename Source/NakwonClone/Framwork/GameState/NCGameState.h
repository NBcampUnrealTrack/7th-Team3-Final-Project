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
	
	UFUNCTION(BlueprintCallable)
	void AddThrowerKillCount() {  ThrowerKillCount++ ;}

	//헌호수정 - 게임 종료 점수판용 타입별 킬 카운트 getter
	UFUNCTION(BlueprintCallable)
	int32 GetWalkerKillCount() const { return WalkerKillCount; }

	UFUNCTION(BlueprintCallable)
	int32 GetRunnerKillCount() const { return RunnerKillCount; }

	UFUNCTION(BlueprintCallable)
	int32 GetWitchKillCount() const { return WitchKillCount; }

	UFUNCTION(BlueprintCallable)
	int32 GetTankKillCount() const { return TankKillCount; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetThrowerKillCount() const { return ThrowerKillCount; }

	//헌호수정 - 전체 좀비 처치 수 (타입 합계)
	UFUNCTION(BlueprintCallable)
	int32 GetTotalKillCount() const { return WalkerKillCount + RunnerKillCount + WitchKillCount + TankKillCount + ThrowerKillCount; }

private:

	//헌호수정 - Replicated로 변경: 멀티에서 모든 클라 점수판에 킬 수 정상 표시 (서버에서만 세지므로 복제 필요)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 WalkerKillCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 RunnerKillCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 WitchKillCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 TankKillCount;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Score", meta = (AllowPrivateAccess = "true"))
	int32 ThrowerKillCount;
};
