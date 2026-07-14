// Fill out your copyright notice in the Description page of Project Settings.


#include "NCInvincibilityItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerController/NCPlayerController.h"

ANCInvincibilityItemActor::ANCInvincibilityItemActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANCInvincibilityItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	Player->ActivateInvincibility(Duration);

	if (ANCPlayerController* NCPC = Cast<ANCPlayerController>(Player->GetController()))
	{
		NCPC->Client_ShowNotification(FText::FromString(TEXT("무적 상태가 되었습니다")), FLinearColor::Yellow);
	}

	ConsumeItem(Interactor);
}