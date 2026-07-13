// Fill out your copyright notice in the Description page of Project Settings.


#include "NCInvincibilityItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"


ANCInvincibilityItemActor::ANCInvincibilityItemActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANCInvincibilityItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	Player->ActivateInvincibility(Duration);

	ConsumeItem();
}
