// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterWalkerController.h"

#include "BehaviorTree/BlackboardComponent.h"


AVGMonsterWalkerController::AVGMonsterWalkerController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AVGMonsterWalkerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(IsAwakeKey, false);
	}
}

