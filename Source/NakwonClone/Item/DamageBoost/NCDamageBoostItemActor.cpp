// Fill out your copyright notice in the Description page of Project Settings.


#include "NCDamageBoostItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCGunComponent.h"
#include "Player/PlayerController/NCPlayerController.h"

void ANCDamageBoostItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	if (UNCGunComponent* GunComp = Player->FindComponentByClass<UNCGunComponent>())
	{
		GunComp->ActivateDamageBoost(Multiplier, Duration);
	}

	if (ANCPlayerController* NCPC = Cast<ANCPlayerController>(Player->GetController()))
	{
		NCPC->Client_ShowNotification(FText::FromString(TEXT("공격력이 강화되었습니다")), FLinearColor::Red);
	}

	ConsumeItem(Interactor);
}