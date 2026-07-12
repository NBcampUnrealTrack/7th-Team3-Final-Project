// Fill out your copyright notice in the Description page of Project Settings.


#include "NCShopGunItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"

void ANCShopGunItemActor::OnPurchased(ANCPlayerCharacter* Player)
{
	if (GunID.IsNone()) return;

	UNCEquipmentComponent* EquipComp = Player->GetEquipmentComponent();
	if (!EquipComp) return;

	if (!EquipComp->IsSwapping())
	{
		EquipComp->DropOccupantGunForNewGun(GunID, GetActorLocation(), GetActorRotation(), OldGunDropLifeSpan);
	}

	EquipComp->EquipGun(GunID);
}