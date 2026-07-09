// Fill out your copyright notice in the Description page of Project Settings.


#include "NCGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


void ANCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCGameState, ConnectedPlayerCount);
	DOREPLIFETIME(ANCGameState, AlivePlayerCount);
	DOREPLIFETIME(ANCGameState, CurrentGameStateTag);
	DOREPLIFETIME(ANCGameState, RemainingMatchTime);
	DOREPLIFETIME(ANCGameState, TotalScore);
	DOREPLIFETIME(ANCGameState, bEscapable);
}

void ANCGameState::OnRep_TotalScore(int32 OldTotalScore)
{
	OnScoreChanged.Broadcast(TotalScore, TotalScore - OldTotalScore);
}

void ANCGameState::OnRep_bEscapable()
{
	// todo : 탈출 가능 안내 위젯 바인딩
}

void ANCGameState::Multicast_PlayHelicopterSound_Implementation(USoundBase* Sound)
{
	UGameplayStatics::PlaySound2D(this, Sound);
}

void ANCGameState::OnRep_CurrentGameStateTag()
{
	// todo : UI갱신?
}

void ANCGameState::OnRep_MatchTime()
{
	// todo : 타이머 UI 갱신
	OnRemainingMatchTimeUpdate.Broadcast(RemainingMatchTime);
}
