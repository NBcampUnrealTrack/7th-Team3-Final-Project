// Fill out your copyright notice in the Description page of Project Settings.


#include "NCGameMode.h"
#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "NakwonClone/Player/PlayerController/NCPlayerController.h"

#include "NakwonClone/Framwork/GameState/NCGameState.h"
#include "NakwonClone/Framwork/GameInstacne/NCGameInstance.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"

ANCGameMode::ANCGameMode()
{
	GameStateClass = ANCGameState::StaticClass();
	PlayerStateClass = ANCPlayerState::StaticClass();
	PlayerControllerClass = ANCPlayerController::StaticClass();
	
}

void ANCGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UNCGameInstance* GI = Cast<UNCGameInstance>(GetGameInstance()))
	{
		GI->StartPreloading();
	}

	ANCGameState* GS = GetGameState<ANCGameState>();
	GS->RemainingMatchTime = 900; // 15분
}

void ANCGameMode::AddPoints(int32 Points)
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	ANCPlayerCharacter* PC = Cast<ANCPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
	if (!GS) return;
	if (!PC) return;

	const int32 OldScore = GS->TotalScore;
	int32 ComboPoints = Points * PC->GetCurrentComboMultiplier();
	GS->TotalScore += ComboPoints;
	GS->OnRep_TotalScore(OldScore);

	CheckPoints();
}

void ANCGameMode::CheckPoints()
{
	ANCGameState* GS = GetGameState<ANCGameState>();
	if (!GS) return;

	if (!bSpecialZombieSpawned && GS->TotalScore >= SpecialZombieScore && GS->TotalScore < EscapableScore)
	{
		bSpecialZombieSpawned = true;
		
		// todo : 특수 좀비 스폰
	}
	else if (!GS->bEscapable && GS->TotalScore >= EscapableScore)
	{
		GS->bEscapable = true;
		UE_LOG(LogTemp, Error, TEXT("Escapable score checked"));
		UE_LOG(LogTemp, Error, TEXT("CheckPoints: TotalScore=%d, EscapableScore=%d, bEscapable=%d"),
        	GS->TotalScore, EscapableScore, GS->bEscapable);

		GS->Multicast_PlayHelicopterSound(HelicopterSound);
		GS->OnRep_bEscapable();
		
		HandleMatchEnd(true);
	}
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
}

void ANCGameMode::MoveToTitle()
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	
	GetWorld()->ServerTravel("/Game/Maps/L_TitleAndLobby?listen"); // todo : 타이틀 경로 추가 예시-("/Game/Maps/TitleMap?listen")
}

void ANCGameMode::MoveToLobby()
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	
	GetWorld()->ServerTravel("/Game/Maps/Lobby/L_Lobby?listen");
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
	
	GS->OnRemainingMatchTimeUpdate.Broadcast(GS->RemainingMatchTime);
	
	if (GS->RemainingMatchTime <= 0.f)
	{
		GetWorldTimerManager().ClearTimer(MatchTimerHandle);
		HandleMatchEnd(false);
	}
}
