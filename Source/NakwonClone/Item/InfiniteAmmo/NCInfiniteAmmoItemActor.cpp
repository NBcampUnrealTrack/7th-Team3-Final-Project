// Fill out your copyright notice in the Description page of Project Settings.

#include "NCInfiniteAmmoItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"
#include "Player/PlayerComponent/NCGunComponent.h"

void ANCInfiniteAmmoItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	UNCEquipmentComponent* EquipComp = Player->GetEquipmentComponent();
	if (!EquipComp) return;

	UNCGunComponent* ActiveWeapon = EquipComp->GetActiveWeapon();
	if (!ActiveWeapon) return; // 총 미장착 시 아이템 소모 안 함

	ActiveWeapon->ActivateInfiniteAmmo(Duration);

	ConsumeItem();
}