
#include "NCOnlineSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Kismet/GameplayStatics.h"

void UNCOnlineSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	// 친구 초대 수락 콜백 등록 (게임 실행 내내 유지)
	Sessions->OnSessionUserInviteAcceptedDelegates.AddUObject(
		this, &UNCOnlineSessionSubsystem::OnInviteAccepted);
}

void UNCOnlineSessionSubsystem::Deinitialize()
{
	DestroyCurrentSession();
	
	Super::Deinitialize();
}

void UNCOnlineSessionSubsystem::HostSession(int32 NumPublicConnections)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->DestroySession(NAME_GameSession);
	
	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = false;
	Settings.NumPublicConnections = NumPublicConnections;
	Settings.bAllowJoinInProgress = false; // 게임 시작 후 참가 불가
	Settings.bShouldAdvertise = true;      // 스팀에 방 검색 허용
	Settings.bUsesPresence = true;         // 친구 목록 표시
	Settings.bAllowInvites = true;         // 친구 초대 허용
	Settings.bAllowJoinViaPresence = true; // 친구 통해 참가 허용
	Settings.bUseLobbiesIfAvailable = true;// 스팀 로비 API 사용
	
	Sessions->OnCreateSessionCompleteDelegates.AddUObject(
		this,
		&UNCOnlineSessionSubsystem::OnCreateSessionComplete);
	
	Sessions->CreateSession(0, NAME_GameSession, Settings);
}

void UNCOnlineSessionSubsystem::FindSessions(int32 MaxResult)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	LastSearch = MakeShareable(new FOnlineSessionSearch());
	LastSearch->MaxSearchResults = MaxResult;
	LastSearch->bIsLanQuery = false;
	
	// steam presence 기반으로마나 검색(열려있는 방만 뜨도록)
	LastSearch->QuerySettings.Set(
		FName(TEXT("PRESENCESEARCH")),
		true,
		EOnlineComparisonOp::Equals);
	
	Sessions->OnFindSessionsCompleteDelegates.AddUObject(
		this,
		&UNCOnlineSessionSubsystem::OnFindSessionsComplete);
	
	Sessions->FindSessions(0, LastSearch.ToSharedRef());
}

void UNCOnlineSessionSubsystem::JoinSessionByIndex(int32 Index)
{
	if (!LastSearchResults.IsValidIndex(Index)) return;
	
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->OnJoinSessionCompleteDelegates.AddUObject(
		this,
		&UNCOnlineSessionSubsystem::OnJoinSessionComplete);
	
	Sessions->JoinSession(
		0,
		NAME_GameSession,
		LastSearchResults[Index]);
}

void UNCOnlineSessionSubsystem::DestroyCurrentSession()
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session) return;
	
	Session->OnDestroySessionCompleteDelegates.AddUObject(
		this,
		&UNCOnlineSessionSubsystem::OnDestroySessionComplete);
	
	Session->DestroySession(NAME_GameSession);
}

FString UNCOnlineSessionSubsystem::GetSessionOwnerName(int32 Index) const
{
	if (!LastSearchResults.IsValidIndex(Index))
	{
		return TEXT("");
	}

	return LastSearchResults[Index].Session.OwningUserName;
}

int32 UNCOnlineSessionSubsystem::GetSessionOpenConnections(int32 Index) const
{
	if (!LastSearchResults.IsValidIndex(Index))
	{
		return 0;
	}

	return LastSearchResults[Index].Session.NumOpenPublicConnections;
}

void UNCOnlineSessionSubsystem::ShowInviteOverlay()
{
	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
	if (!OSS) return;
	
	IOnlineExternalUIPtr ExternalUI = OSS->GetExternalUIInterface();
	if (!ExternalUI) return;

	// Steam 오버레이의 친구 초대 창 열기
	ExternalUI->ShowInviteUI(0, NAME_GameSession);
}

void UNCOnlineSessionSubsystem::LoadFriendsList()
{
	IOnlineFriendsPtr Friends = GetFriendsInterface();
	if (!Friends) return;

	Friends->ReadFriendsList(
		0,
		TEXT("default"),
		FOnReadFriendsListComplete::CreateUObject(
			this, &UNCOnlineSessionSubsystem::OnReadFriendsListComplete));
}

void UNCOnlineSessionSubsystem::SendInviteToFriend(int32 FriendIndex)
{
	if (!CachedFriends.IsValidIndex(FriendIndex)) return;

	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;

	TSharedRef<FOnlineFriend> Friend = CachedFriends[FriendIndex];
	Sessions->SendSessionInviteToFriend(
		0,
		NAME_GameSession,
		*Friend->GetUserId());

	OnInviteSentEvent.Broadcast();
}

FString UNCOnlineSessionSubsystem::GetFriendName(int32 Index) const
{
	if (!CachedFriends.IsValidIndex(Index)) return TEXT("");
	return CachedFriends[Index]->GetRealName();
}

bool UNCOnlineSessionSubsystem::IsFriendOnline(int32 Index) const
{
	if (!CachedFriends.IsValidIndex(Index)) return false;

	const FOnlineUserPresence& Presence = CachedFriends[Index]->GetPresence();
	return Presence.bIsOnline || Presence.bIsPlaying;
}

IOnlineFriendsPtr UNCOnlineSessionSubsystem::GetFriendsInterface() const
{
	IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld()); // ??????????
	if (!OSS) return nullptr;
	
	return OSS->GetFriendsInterface();
}

void UNCOnlineSessionSubsystem::OnReadFriendsListComplete(
	int32 LocalUserNum,
	bool bWasSuccessful,
	const FString& ListName,
	const FString& ErrorStr)
{
	CachedFriends.Empty();

	if (bWasSuccessful)
	{
		IOnlineFriendsPtr Friends = GetFriendsInterface();
		if (Friends)
		{
			Friends->GetFriendsList(0, TEXT("default"), CachedFriends);
		}
	}

	OnFriendsListLoadedEvent.Broadcast(bWasSuccessful, CachedFriends.Num());
}

IOnlineSessionPtr UNCOnlineSessionSubsystem::GetSessionInterface() const
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		return nullptr;
	}

	return OnlineSubsystem->GetSessionInterface();
}

void UNCOnlineSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->ClearOnCreateSessionCompleteDelegates(this);
	
	if (bSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel("Game/Maps/L_MVP?listen");
		}
	}
}

void UNCOnlineSessionSubsystem::OnFindSessionsComplete(bool bSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->ClearOnFindSessionsCompleteDelegates(this);
	
	if (bSuccessful && LastSearch.IsValid())
	{
		// 결과 캐싱 -> UI 에서 인덱스로 접근
		LastSearchResults = LastSearch->SearchResults;
	}
	
	// 블루프린트 UI에 완료 알림
	OnFindSessionsCompleteEvent.Broadcast(bSuccessful && LastSearchResults.Num() > 0);
}

void UNCOnlineSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->ClearOnJoinSessionCompleteDelegates(this);
	
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// 스팀에서 접속 주소를 받아서 이동
		FString TravelURL;
		Sessions->GetResolvedConnectString(SessionName, TravelURL);
		
		APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
		if (!PC) return;
		
		PC->ClientTravel(TravelURL, TRAVEL_Absolute);
		
		OnJoinSessionCompleteEvent.Broadcast(true);
	}
	else
	{
		OnJoinSessionCompleteEvent.Broadcast(false);		
	}
}

void UNCOnlineSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bSuccessful)
{
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->ClearOnDestroySessionCompleteDelegates(this);
	
	// 세션 파괴 후 검색 결과 초기화
	LastSearchResults.Empty();
	LastSearch.Reset();
}

void UNCOnlineSessionSubsystem::OnInviteAccepted(
	bool bSuccessful, 
	int32 LocalUserNum,
	TSharedPtr<const FUniqueNetId> UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	if (!bSuccessful) return;
	
	IOnlineSessionPtr Sessions = GetSessionInterface();
	if (!Sessions) return;
	
	Sessions->OnJoinSessionCompleteDelegates.AddUObject(
		this,
		&UNCOnlineSessionSubsystem::OnJoinSessionComplete);
	
	Sessions->JoinSession(0, NAME_GameSession, InviteResult);
}
