// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Sleep.h"

#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Sleep::UBTTask_Sleep()
{
	NodeName = "Sleep";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Sleep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Monster = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	Monster->PlayAnimMontage(Monster->GetSleepMontage());

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_Sleep::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Monster = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster)
	{
		Monster->StopAnimMontage(Monster->GetSleepMontage());
		Monster->PlayAnimMontage(Monster->GetSelectedWakeUpMontage());
	}

	return EBTNodeResult::Aborted;
}
