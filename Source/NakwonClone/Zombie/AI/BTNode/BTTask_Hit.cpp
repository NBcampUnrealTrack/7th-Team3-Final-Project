// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Hit.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_Hit::UBTTask_Hit()
{
	NodeName = TEXT("Hit");
}

EBTNodeResult::Type UBTTask_Hit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AIController->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Hit::OnMontageEnded);

	return EBTNodeResult::InProgress;
}

void UBTTask_Hit::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster)
	{
		UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Hit::OnMontageEnded);
		}
	}

	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Hit::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CachedOwnerComp)
	{
		// BIsHit 초기화
		if (UBlackboardComponent* Blackboard = CachedOwnerComp->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(FName("BIsHit"), false);
		}

		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		CachedOwnerComp = nullptr;
	}
}
