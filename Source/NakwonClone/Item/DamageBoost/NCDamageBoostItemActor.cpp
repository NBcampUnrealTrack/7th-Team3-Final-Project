// Fill out your copyright notice in the Description page of Project Settings.


#include "NCDamageBoostItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCGunComponent.h"

void ANCDamageBoostItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	if (UNCGunComponent* GunComp = Player->FindComponentByClass<UNCGunComponent>())
	{
		GunComp->ActivateDamageBoost(Multiplier, Duration);
	}

	ConsumeItem();
}
