// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
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
	~ANCGameMode();
	
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController *NewPlayer) override;
	
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void JoinSession();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void LeaveSession();
	
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void MoveToTitle();
	UFUNCTION(BlueprintCallable, Category = "GameFlow")
	void MoveToLobby();
	
	void InviteFriend(APlayerController* TargetPlayer);
	void KickFriend(APlayerController* TargetPlayer);

	void HandlePlayerDowned();  // todo : 플레이어 스테이트 정의하고 파라미터에 넣기
	void HandlePlayerRevived(); // todo : 플레이어 스테이트 정의하고 파라미터에 넣기
	
	FTimerHandle MatchTimerHandle;
	
	UFUNCTION(BlueprintCallable)
	void SetMatchTimerHandle();

private:
	bool bInviteFriend = false;
	
	bool bJoinSessionSingle = false;
	bool bJoinSessionDuo = false;
	
	bool bStartSession = false;
};
