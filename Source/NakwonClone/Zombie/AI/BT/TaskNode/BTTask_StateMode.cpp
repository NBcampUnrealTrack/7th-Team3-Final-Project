// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_StateMode.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTTask_StateMode::UBTTask_StateMode()
{
	NodeName = "State Mode";
}

EBTNodeResult::Type UBTTask_StateMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterAIControllerBase* AICon = Cast<AVGMonsterAIControllerBase>(OwnerComp.GetAIOwner());
	if (!AICon) return EBTNodeResult::Failed;

	return AICon->FindPatrolLocation() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
