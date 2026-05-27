// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_FindPatrolLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_FindPatrolLocation::UBTTask_FindPatrolLocation()
{
	NodeName = "Find Patrol Location";
}

EBTNodeResult::Type UBTTask_FindPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	APawn* Monster = AIController->GetPawn();
	if (!Monster ) return EBTNodeResult::Failed;
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;
	
	FVector Origin = Monster->GetActorLocation();
	float CurrentRadius = SearchRadius;
	
	while (CurrentRadius > 0.f)
	{
		FNavLocation RandomLocation;
		bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, CurrentRadius, RandomLocation);
		
		if (bFound)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(
				AVGMonsterAIControllerBase::PatrolLocationKey,
				RandomLocation.Location);
			
			AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
			if (Walker) Walker->SetMonsterState(EMonsterState::Patrol);
			
			return EBTNodeResult::Succeeded;
		}
		
		CurrentRadius -= ReductionUnit;
	}
	return EBTNodeResult::Failed;
}
