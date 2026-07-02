
#include "LobbyGameMode.h"
#include "Engine/Engine.h"

ALobbyGameMode::ALobbyGameMode()
{
	DefaultPawnClass = nullptr;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<FName> LevelToPreload;
	LevelToPreload.Add(FName("/Game/Maps/지하주차장레벨경로")); // todo : 경로 추가
	GEngine->PrepareMapChange(GetWorld(), LevelToPreload);
	
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 중..."));
	
	// 완료 감지용 타이머
	GetWorldTimerManager().SetTimer(
		NextMapReadyTimerHandle,
		this,
		&ALobbyGameMode::CheckMapReady,
		1.f,
		true
	);
}

void ALobbyGameMode::TryCommitMapChange()
{
	if (GetWorld()->IsMapChangeReady())
	{
		GetWorldTimerManager().ClearTimer(NextMapReadyTimerHandle);
		GEngine->CommitMapChange(GetWorld());
	}
}

void ALobbyGameMode::CheckMapReady()
{
	if (GetWorld()->IsMapChangeReady())
	{
		GetWorldTimerManager().ClearTimer(NextMapReadyTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 완료!"));
	}
}

void ALobbyGameMode::MoveToShopLevel()
{
	if (GetWorld()->IsMapChangeReady())
	{
		GEngine->CommitMapChange(GetWorld());
	}
	else
	{
		GetWorld()->ServerTravel("/Game/Maps/지하주차장레벨이름"); // 프리 로딩 미완료 시 실행
	}
}
