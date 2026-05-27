// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Attack.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;
	
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (Walker) Walker->SetMonsterState(EMonsterState::Attack);
	
	return EBTNodeResult::Succeeded;
}
