// Fill out your copyright notice in the Description page of Project Settings.
#include "BTTask_ChaseMode.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_ChaseMode::UBTTask_ChaseMode()
{
	NodeName = "Move & Run Mode";
}

EBTNodeResult::Type UBTTask_ChaseMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ACharacter* SelfChar = Cast<ACharacter>(AICon->GetPawn());
	if (!SelfChar || !SelfChar->GetCharacterMovement()) return EBTNodeResult::Failed;

	float ChaseSpeed = MoveSpeed;   // 폴백: 좀비가 아니거나 값 없으면 노드 기본값

	if (AVGMonsterCharacterBase* Mon = Cast<AVGMonsterCharacterBase>(SelfChar))
	{
		if (Mon->GetMonsterAttributeSet())
		{
			ChaseSpeed = Mon->GetMonsterAttributeSet()->GetMoveSpeed();  // DT MoveSpeed
		}
	}

	SelfChar->GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;

	return EBTNodeResult::Succeeded;
}