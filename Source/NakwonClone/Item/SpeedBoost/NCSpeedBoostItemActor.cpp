// Fill out your copyright notice in the Description page of Project Settings.

#include "NCSpeedBoostItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/Locomotion/UNCLocomotionComponent.h"
#include "Player/PlayerController/NCPlayerController.h"

void ANCSpeedBoostItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	UNCLocomotionComponent* LocomotionComp = Player->GetLocomotionComponent();
	if (!LocomotionComp) return;

	LocomotionComp->ActivateSpeedBoost(SpeedMultiplier, Duration);

	if (ANCPlayerController* NCPC = Cast<ANCPlayerController>(Player->GetController()))
	{
		NCPC->Client_ShowNotification(FText::FromString(TEXT("이동 속도가 증가했습니다")), FLinearColor::Green);
	}

	ConsumeItem(Interactor);
}