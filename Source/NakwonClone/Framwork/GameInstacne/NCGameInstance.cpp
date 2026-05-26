// Fill out your copyright notice in the Description page of Project Settings.

#include "NCGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

void UNCGameInstance::Init()
{
	Super::Init();

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;

	// 친구가 초대를 수락했을 때 바인딩
	Sessions->OnSessionUserInviteAcceptedDelegates.AddUObject(
		this, &UNCGameInstance::OnInviteAccepted);
}

void UNCGameInstance::CreateListenSession()
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;

	// 기존 세션이 있으면 먼저 제거
	Sessions->DestroySession(NAME_GameSession);

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = false;
	Settings.bUsesPresence = true;		// Steam 친구 목록에 현재 상태 표시 (초대에 필수)
	Settings.bAllowInvites = true;
	Settings.bShouldAdvertise = true;
	Settings.NumPublicConnections = 4;

	Sessions->OnCreateSessionCompleteDelegates.AddUObject(
		this, &UNCGameInstance::OnCreateSessionComplete);

	Sessions->CreateSession(0, NAME_GameSession, Settings);
}

void UNCGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions)
	{
		Sessions->ClearOnCreateSessionCompleteDelegates(this);
	}

	if (bWasSuccessful)
	{
		// 세션 생성 완료 → 리슨 서버로 맵 이동
		GetWorld()->ServerTravel("/Game/NakwonClone/MainLevel?listen");
	}
}

void UNCGameInstance::JoinFoundSession(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;

	Sessions->OnJoinSessionCompleteDelegates.AddUObject(
		this, &UNCGameInstance::OnJoinSessionComplete);

	Sessions->JoinSession(0, NAME_GameSession, SearchResult);
}

void UNCGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (Sessions)
	{
		Sessions->ClearOnJoinSessionCompleteDelegates(this);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString TravelURL;
		Sessions->GetResolvedConnectString(SessionName, TravelURL);

		APlayerController* PC = GetFirstLocalPlayerController();
		if (PC)
		{
			PC->ClientTravel(TravelURL, TRAVEL_Absolute);
		}
	}
}

void UNCGameInstance::OnInviteAccepted(bool bWasSuccessful, int32 LocalUserNum,
	TSharedPtr<const FUniqueNetId> UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful)
	{
		JoinFoundSession(InviteResult);
	}
}

IOnlineSessionPtr UNCGameInstance::GetSessionInterface() const
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (!OSS) return nullptr;

	return OSS->GetSessionInterface();
}
