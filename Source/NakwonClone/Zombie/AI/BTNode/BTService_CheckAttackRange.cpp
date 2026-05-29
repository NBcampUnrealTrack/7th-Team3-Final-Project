// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_CheckAttackRange.h"
#include "AICOntroller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTService_CheckAttackRange::UBTService_CheckAttackRange()
{
	NodeName = "BTService_CheckAttackRange";
	Interval = 0.1f;
}

void UBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UE_LOG(LogTemp, Warning, TEXT("[Service] CheckAttackRange 실행 중"));
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;
	
	APawn* Monster = AIController->GetPawn();
	if (!Monster) return;
	
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;
	
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(
		AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target)
	{
		Blackboard->SetValueAsBool("bIsAttack", false);
		return;
	}
	
	// 거리 계산
	float Distance = FVector::Dist(
		Monster->GetActorLocation(),
		Target->GetActorLocation());
	
	// 공격 범위 안이면 bIsAttack = true
	Blackboard->SetValueAsBool("bIsAttack", Distance <= AttackRange);
}
