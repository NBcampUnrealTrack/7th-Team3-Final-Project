// Fill out your copyright notice in the Description page of Project Settings.


#include "NCGameMode.h"
#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
// #include "NakwonClone/Player/PlayerController/NCPlayerController.h"

#include "NakwonClone/Framwork/GameState/NCGameState.h"

ANCGameMode::ANCGameMode()
{
	GameStateClass = ANCGameState::StaticClass();
	PlayerStateClass = ANCPlayerState::StaticClass();
	// PlayerControllerClass = ANCPlayerController::StaticClass(); // todo : NCPlayerController 맞는지 확인
	
	// 플레이어 기본 캐릭터 지정
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
	if (GS)
	{
		GS->ConnectedPlayerCount--;
	}
}

void ANCGameMode::StartMatch()
{
	Super::StartMatch();
}

void ANCGameMode::EndMatch(bool bClear)
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;

	if (bClear)
	{
		GS->CurrentGameStateTag = NCGameState::GameClear;
	}
	else
	{
		GS->CurrentGameStateTag = NCGameState::GameOver;
	}
}

void ANCGameMode::JoinSession()
{
	// todo : 게임 스테이트의 변수 바꿀것 있는지 확인하기 (GamePhase 등)
	
	if (bStartSession && bJoinSessionSingle)
	{
		// todo : 싱글 플레이(Stand Alone)	
	}
	else if (bStartSession && bJoinSessionDuo)
	{
		// todo : 서버 트레블 실행(Listen Server)
	}
}

void ANCGameMode::LeaveSession() // todo : 게임 페이즈 파라미터 추가
{
	bStartSession = false;
	
	// todo : 게임 스테이트 변수 수정
	
	
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ANCGameMode::MoveToTitle()
{
	// todo : 타이틀로 이동 로직 구현
}

void ANCGameMode::MoveToLobby()
{
	// todo : 로비로 이동 로직 구현
}

void ANCGameMode::InviteFriend(APlayerController* TargetPlayer)
{
}

void ANCGameMode::KickFriend(APlayerController* TargetPlayer)
{
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
}

void ANCGameMode::HandlePlayerDead(ANCPlayerState* PlayerState)
{
	if (!PlayerState) return;

	PlayerState->LifeStateTag = NCCharacter::Dead;

	CheckAllPlayersDead();
}

void ANCGameMode::CheckAllPlayersDead()
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;

	if (GS->AlivePlayerCount <= 0)
	{
		EndMatch(false); // 전멸 → 게임오버
	}
}

void ANCGameMode::SetMatchTimerHandle()
{
}
