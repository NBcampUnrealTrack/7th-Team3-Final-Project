// Fill out your copyright notice in the Description page of Project Settings.


#include "NCGameState.h"

#include "Net/UnrealNetwork.h"

void ANCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCGameState, ConnectedPlayerCount);
	DOREPLIFETIME(ANCGameState, AlivePlayerCount);
	DOREPLIFETIME(ANCGameState, CurrentGameStateTag);
	DOREPLIFETIME(ANCGameState, RemainingMatchTime);
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
