// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RequestWaitSlot.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_RequestWaitSlot::UBTTask_RequestWaitSlot()
{
	NodeName = "Request Wait Slot";
}

EBTNodeResult::Type UBTTask_RequestWaitSlot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AICon->GetPawn());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Monster || !BB) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target) return EBTNodeResult::Failed;

	FVector SlotLocation;
	if (!Monster->ReserveWaitSlot(Target, SlotLocation)) return EBTNodeResult::Failed;

	BB->SetValueAsVector(AVGMonsterAIControllerBase::SlotLocationKey, SlotLocation);
	return EBTNodeResult::Succeeded;
}