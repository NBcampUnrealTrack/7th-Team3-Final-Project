// Fill out your copyright notice in the Description page of Project Settings.


#include "NCPlayerState.h"

#include "Net/UnrealNetwork.h"

void ANCPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCPlayerState, LifeStateTag);
	DOREPLIFETIME(ANCPlayerState, CurrentHP);
	DOREPLIFETIME(ANCPlayerState, MaxHP);
	DOREPLIFETIME(ANCPlayerState, bHost);
	DOREPLIFETIME(ANCPlayerState, TeamIndex);
}

void ANCPlayerState::OnRep_LifeStateTag()
{
	// todo : UI 갱신 로직 추가
}
