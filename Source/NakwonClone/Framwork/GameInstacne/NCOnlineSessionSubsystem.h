// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSessionSettings.h"
#include "NCOnlineSessionSubsystem.generated.h"

// 블루프린트에서 받을 수 있는 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNCOnFindSessionsComplete, bool, bSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNCOnJoinSessionComplete, bool, bSuccessful);

UCLASS()
class NAKWONCLONE_API UNCOnlineSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// host
	UFUNCTION(BlueprintCallable, Category = "Session")
	void HostSession(int32 NumPublicConnections = 2);
	
	// client - Search session list
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions(int32 MaxResult = 20);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSessionByIndex(int32 Index);
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroyCurrentSession();
	
	// bp UI에서 검색 완료 이벤트를 받을 때 사용
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FNCOnFindSessionsComplete OnFindSessionsCompleteEvent;
	
	UPROPERTY(BlueprintAssignable, Category = "Session")
	FNCOnJoinSessionComplete OnJoinSessionCompleteEvent;
	
	// UI에서 방 목록 표시용 - host 이름, 인원
	UFUNCTION(BlueprintCallable, Category = "Session")
	int32 GetSearchResultCount() const { return LastSearchResults.Num(); }
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	FString GetSessionOwnerName(int32 Index) const;
	
	UFUNCTION(BlueprintCallable, Category = "Session")
	int32 GetSessionOpenConnections(int32 Index) const;
	
private:
	IOnlineSessionPtr GetSessionInterface() const;
	
	// steam이 자동 호출하는 함수들
	void OnCreateSessionComplete(FName SessionName, bool bSuccessful);
	void OnFindSessionsComplete(bool bSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bSuccessful);
	void OnInviteAccepted(
		bool bSuccessful,
		int32 LocalUserNum,
		TSharedPtr<const FUniqueNetId> UserId,
		const FOnlineSessionSearchResult& InviteResult);
	
	// 검색 결과
	TSharedPtr<FOnlineSessionSearch> LastSearch;
	TArray<FOnlineSessionSearchResult> LastSearchResults;
};
