// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdateRotation.h"
#include "AIController.h"

UBTService_UpdateRotation::UBTService_UpdateRotation()
{
	NodeName = "Update Rotation";
	
	// BT Service 실행 간격 (초)
	Interval = 0.3f;
}

void UBTService_UpdateRotation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	
	if (!AIController || !AIController->GetPawn())
	{
		return;
	}
	
	AIController->SetControlRotation(
		AIController->GetPawn()->GetActorRotation());
}