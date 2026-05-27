// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Dead.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Dead::UBTTask_Dead()
{
	NodeName = "Dead";
}

EBTNodeResult::Type UBTTask_Dead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	Monster->Dead();

	// BT 중단
	OwnerComp.StopTree(EBTStopMode::Safe);

	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (Walker) Walker->SetMonsterState(EMonsterState::Dead);
	
	return EBTNodeResult::Succeeded;
}
