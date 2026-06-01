// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_CheckAttackRange.h"
#include "AICOntroller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogAttackRange, Log, All);

UBTService_CheckAttackRange::UBTService_CheckAttackRange()
{
	NodeName = "BTService_CheckAttackRange";
	Interval = 0.1f;
}

void UBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UE_LOG(LogAttackRange, Warning, TEXT("[Service] CheckAttackRange 실행 중"));
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogAttackRange, Error, TEXT("[Service] AIController 없음"));
		return;
	}
	
	APawn* Monster = AIController->GetPawn();
	if (!Monster)
	{
		UE_LOG(LogAttackRange, Error, TEXT("[Service] Monster 없음"));
		return;
	}
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		UE_LOG(LogAttackRange, Error, TEXT("[Service] BB 없음"));
		return;
	}
	
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(
		AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target)
	{
		UE_LOG(LogAttackRange, Error, TEXT("[Service] Target 없음"));
		Blackboard->SetValueAsBool("bIsAttack", false);
		return;
	}
	
	// 거리 계산
	float Distance = FVector::Dist(
		Monster->GetActorLocation(),
		Target->GetActorLocation());

	UE_LOG(LogAttackRange, Warning, TEXT("[Service] Distance: %f, AttackRange: %f, Result: %s"),
		Distance, AttackRange, Distance <= AttackRange ? TEXT("true") : TEXT("false"));
	
	// 공격 범위 안이면 bIsAttack = true
	Blackboard->SetValueAsBool("bIsAttack", Distance <= AttackRange);
}
