// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ClearValue.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearValue::UBTTask_ClearValue()
{
	NodeName = TEXT("Clear Value");
}

void UBTTask_ClearValue::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		BlackboardKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_ClearValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blacboard = OwnerComp.GetBlackboardComponent();
	if (!Blacboard)
	{
		return EBTNodeResult::Failed;
	}
	
	Blacboard->ClearValue(BlackboardKey.GetSelectedKeyID());
	
	return EBTNodeResult::Succeeded;
}
