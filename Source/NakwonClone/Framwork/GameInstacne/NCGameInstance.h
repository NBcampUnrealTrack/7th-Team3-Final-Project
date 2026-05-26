// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NCGameInstance.generated.h"

UCLASS()
class NAKWONCLONE_API UNCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateListenSession();

	// 세션 입장 (클라이언트가 초대 수락 시 호출)
	void JoinFoundSession(const FOnlineSessionSearchResult& SearchResult);

private:
	// 세션 생성 완료 콜백
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	// 세션 입장 완료 콜백
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	// 친구 초대 수락 콜백
	void OnInviteAccepted(bool bWasSuccessful, int32 LocalUserNum,
		TSharedPtr<const FUniqueNetId> UserId,
		const FOnlineSessionSearchResult& InviteResult);

	IOnlineSessionPtr GetSessionInterface() const;
};
