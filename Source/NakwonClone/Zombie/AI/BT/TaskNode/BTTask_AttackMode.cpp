// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_AttackMode.h"
#include "AIController.h" //헌호수정 - 거리 직접 계산
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTTask_AttackMode::UBTTask_AttackMode()
{
	NodeName = "Attack Mode";
}

EBTNodeResult::Type UBTTask_AttackMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!BB || !AICon) return EBTNodeResult::Failed;

	//헌호수정 - 타겟까지 거리를 직접 계산 (BB Distance 키에 의존 안 함 → 서비스 미갱신/키 오타로 인한 "안 움직임" 버그 방지)
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey));
	APawn* Self = AICon->GetPawn();
	if (!Target || !Self)
	{
		// 타겟 없으면 공격 불가 → 추격으로 폴백
		BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsAttackKey, false);
		return EBTNodeResult::Failed;
	}

	const float Distance = FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation());

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
