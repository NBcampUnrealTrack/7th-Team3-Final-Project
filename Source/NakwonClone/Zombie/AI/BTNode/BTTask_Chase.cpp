// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Chase.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Chase::UBTTask_Chase()
{
	NodeName = "Chase";
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Walker) return EBTNodeResult::Failed;
	
	Walker->SetMonsterState(EMonsterState::Chase);
	
	return EBTNodeResult::Succeeded;
}
