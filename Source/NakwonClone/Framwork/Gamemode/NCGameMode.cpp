// Fill out your copyright notice in the Description page of Project Settings.


#include "NCGameMode.h"
#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "NakwonClone/Player/PlayerController/NCPlayerController.h"

#include "NakwonClone/Framwork/GameState/NCGameState.h"

ANCGameMode::ANCGameMode()
{
	GameStateClass = ANCGameState::StaticClass();
	PlayerStateClass = ANCPlayerState::StaticClass();
	PlayerControllerClass = ANCPlayerController::StaticClass();
	
	// todo : 플레이어 기본 캐릭터 지정
}

ANCGameMode::~ANCGameMode()
{
}

void ANCGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// todo : 아이템, 좀비 스폰 포인트 추가 (배열, UGamePlayStatics::GetAllActorsOfClass(~~))
	/* 월드의 SpawnPoint를 순회하면서 스폰 로직 활성화
	TArray<AActor*> ItemFoundVolumes;
	TArray<AActor*> ZombieFoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AItemSpawnPoint::StaticClass(), ItemFoundVolumes);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieSpawnPoint::StaticClass(), ZombieFoundVolumes);
	
	for (AActor* Actor : ItemFoundVolumes)
	{
		AItemSpawnPoint* ItemSpawnVolume = Cast<AItemSpawnPoint>(Actor);
		if (ItemSpawnVolume)
		{
			ItemSpawnVolume->SpawnItems();
		}
	}
	
	for (AActor* Actor : ZombieFoundVolumes)
	{
		AZombieSpawnPoint* ZombieSpawnVolume = Cast<AZombieSpawnPoint>(Actor);
		if (ZombieSpawnVolume)
		{
			ZombieSpawnVolume->SpawnZombie(5, 10, 4, 6, 1, 1);
		}
	}
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACH4PlayerController* PC = Cast<ACH4PlayerController>(It->Get());
		PC->Client_EnablePlayerInput();
	}
	*/
}

void ANCGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	ANCGameState* GS = Cast<ANCGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->ConnectedPlayerCount++;
		GS->AlivePlayerCount++;
	}
	
	ANCPlayerState* PS = NewPlayer->GetPlayerState<ANCPlayerState>();
	if (PS)
	{
		PS->MaxHP = 100.f;
		PS->CurrentHP = PS->MaxHP;
		PS->LifeStateTag = NCCharacter::Alive;
		PS->bHost = (GS && GS->ConnectedPlayerCount == 1);
	}
}

void ANCGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ANCGameState* GS = GetGameState<ANCGameState>();
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (!GS || !PC) return;
	
	GS->ConnectedPlayerCount--;
	
	ANCPlayerState* PS = PC->GetPlayerState<ANCPlayerState>();
	if (PS && PS->LifeStateTag == NCCharacter::Alive)
	{
		GS->AlivePlayerCount--;
		CheckAllPlayersDead();
	}
}

void ANCGameMode::StartMatch()
{
	Super::StartMatch();
	
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;
	
	GS->CurrentGameStateTag = NCGameStateTags::GameStart;
	
	SetMatchTimerHandle();
}

void ANCGameMode::HandleMatchEnd(bool bClear)
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;

	if (bClear)
	{
		GS->CurrentGameStateTag = NCGameStateTags::GameClear;
	}
	else
	{
		GS->CurrentGameStateTag = NCGameStateTags::GameOver;
	}
	
	// PlayerController 작업 완료 후 추가 예정
	// 모든 플레이어에게 게임오버/클리어 알림
	// for (FConstPlayerControllerIterator It = ...) { PC->Client_OnGameEnd(bClear); }
}

void ANCGameMode::MoveToTitle()
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	
	GetWorld()->ServerTravel("/Game/Maps/L_TitleAndLobby?listen"); // todo : 타이틀 경로 추가 예시-("/Game/Maps/TitleMap?listen")
}

void ANCGameMode::MoveToLobby()
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	
	GetWorld()->ServerTravel("/Game/Maps/L_TitleAndLobby?listen");
}

void ANCGameMode::HandlePlayerDowned(ANCPlayerState* PlayerState)
{
	if (!PlayerState) return;
	
	PlayerState->LifeStateTag = NCCharacter::Downed;
	
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (GS)
	{
		GS->AlivePlayerCount--;
	}

	// todo : PlayerController 작업 완료 후 추가 예정
	// ANCPlayerController* PC = Cast<ANCPlayerController>(PlayerState->GetOwner());
	// if (PC) PC->Client_OnPlayerDowned();
	
	CheckAllPlayersDead();
}

void ANCGameMode::HandlePlayerRevived(ANCPlayerState* PlayerState)
{
	if (!PlayerState) return;

	PlayerState->LifeStateTag = NCCharacter::Alive;
	PlayerState->CurrentHP = PlayerState->MaxHP * 0.5f; // 50% HP로 부활

	ANCGameState* GS = GetGameState<ANCGameState>();
	if (GS)
	{
		GS->AlivePlayerCount++;
	}
	// todo : 소생 구현 함수 호출
}

void ANCGameMode::HandlePlayerDead(ANCPlayerState* PlayerState)
{
	if (!PlayerState) return;

	PlayerState->LifeStateTag = NCCharacter::Dead;

	CheckAllPlayersDead();
	// todo : 사망 구현 함수(팀원 시점 관전)
}

void ANCGameMode::CheckAllPlayersDead()
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;

	if (GS->AlivePlayerCount <= 0)
	{
		HandleMatchEnd(false); // 전멸 → 게임오버
	}
}

void ANCGameMode::SetMatchTimerHandle()
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;

	GS->RemainingMatchTime = 900.f; // 15분
	
	GetWorldTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&ANCGameMode::TimerTick,
		1.f,
		true // 반복 여부
	);
}

void ANCGameMode::TimerTick()
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;
	
	GS->RemainingMatchTime--;
	
	if (GS->RemainingMatchTime <= 0.f)
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		HandleMatchEnd(false);
	}
}
