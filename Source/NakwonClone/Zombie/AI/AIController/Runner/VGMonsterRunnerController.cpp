// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterRunnerController.h"
#include "BehaviorTree/BlackboardComponent.h"


AVGMonsterRunnerController::AVGMonsterRunnerController()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AVGMonsterRunnerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(IsAwakeKey, true);
	}
}



