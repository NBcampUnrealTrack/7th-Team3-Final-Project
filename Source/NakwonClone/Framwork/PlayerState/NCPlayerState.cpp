// Fill out your copyright notice in the Description page of Project Settings.


#include "NCPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "NakwonClone/Common/NCGameplayTags.h"

ANCPlayerState::ANCPlayerState()
{
	MaxHP = 100.f;
	CurrentHP = MaxHP;
	MaxStamina = 100.f;
	CurrentStamina = MaxStamina;
	
	LifeStateTag = NCCharacter::Alive;
}

void ANCPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCPlayerState, LifeStateTag);
	DOREPLIFETIME(ANCPlayerState, CurrentHP);
	DOREPLIFETIME(ANCPlayerState, MaxHP);
	DOREPLIFETIME(ANCPlayerState, CurrentStamina);
	DOREPLIFETIME(ANCPlayerState, MaxStamina);
	DOREPLIFETIME(ANCPlayerState, bHost);
	DOREPLIFETIME(ANCPlayerState, TeamIndex);
}

void ANCPlayerState::OnRep_LifeStateTag()
{
	OnLifeStateChanged.Broadcast(LifeStateTag);
}

void ANCPlayerState::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void ANCPlayerState::OnRep_CurrentStemina()
{
	OnHPChanged.Broadcast(CurrentStamina, MaxStamina);
}
