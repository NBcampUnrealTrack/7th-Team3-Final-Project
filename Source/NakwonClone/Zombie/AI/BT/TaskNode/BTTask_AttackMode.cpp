// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_AttackMode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTTask_AttackMode::UBTTask_AttackMode()
{
	NodeName = "Attack Mode";
}

EBTNodeResult::Type UBTTask_AttackMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	// 서비스가 저장해둔 거리 읽기 (재계산 안 함)
	const float Distance = BB->GetValueAsFloat(AVGMonsterAIControllerBase::DistanceKey);

	if (Distance <= AttackRange)
	{
		// 사거리 안 → 공격 가능
		BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsAttackKey, true);
		return EBTNodeResult::Succeeded;   // Attack Seq 진행 → Attack State 데코로
	}

	// 사거리 밖 → 공격 불가
	BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsAttackKey, false);
	return EBTNodeResult::Failed;          // Attack Seq 실패 → Combat Selector가 Chase로 폴백
}
