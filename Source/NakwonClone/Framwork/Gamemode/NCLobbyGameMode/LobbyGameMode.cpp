
#include "LobbyGameMode.h"
#include "Engine/Engine.h"

ALobbyGameMode::ALobbyGameMode()
{
	DefaultPawnClass = nullptr;
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<FName> LevelToPreload;
	LevelToPreload.Add(FName("/Game/Maps/L_Parking"));
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

void ALobbyGameMode::CheckMapReady()
{
	if (GetWorld()->IsMapChangeReady())
	{
		GetWorldTimerManager().ClearTimer(NextMapReadyTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 완료!"));
	}
}

void ALobbyGameMode::MoveToParkingLevel()
{
	// if (GetWorld()->IsMapChangeReady())
	// {
	// 	GEngine->CommitMapChange(GetWorld());
	// }
	// else
	// {
		GetWorld()->ServerTravel("/Game/Maps/L_Parking");
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 실패로 서버트레블 실행"));
	// }
}
