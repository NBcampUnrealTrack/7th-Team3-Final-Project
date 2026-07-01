// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AICon->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	// TODO: 몸체 담당자 공격 함수 호출
	// Monster->PerformAttack();   // ← 공격 함수 (미구현, 담당자 협업 후 연결)

	return EBTNodeResult::Succeeded;
}
