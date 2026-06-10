// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Hit.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Hit::UBTTask_Hit()
{
	NodeName = TEXT("Hit");
}

EBTNodeResult::Type UBTTask_Hit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	AVG
	
	return EBTNodeResult::Succeeded;
}
