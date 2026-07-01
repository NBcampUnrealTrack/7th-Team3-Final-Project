// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ClearValue.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearValue::UBTTask_ClearValue()
{
	NodeName = "Clear Value";
}

void UBTTask_ClearValue::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// 이거 안 하면 SelectedKeyName이 무효 → ClearValue가 조용히 아무것도 안 지움
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		BlackboardKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_ClearValue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	BB->ClearValue(BlackboardKey.SelectedKeyName);
	return EBTNodeResult::Succeeded;
}
