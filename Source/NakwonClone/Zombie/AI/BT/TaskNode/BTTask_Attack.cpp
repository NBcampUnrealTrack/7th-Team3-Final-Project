// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(AICon->GetPawn());
	if (!Walker) return EBTNodeResult::Failed;

	return Walker->Attack() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
