// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ChaseMode.h"
#include "AIController.h"
#include "GameFramework/Character.h"
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

	SelfChar->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	return EBTNodeResult::Succeeded;
}
