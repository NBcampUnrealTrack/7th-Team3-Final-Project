// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Bite.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Runner/VGMonsterRunner.h"

UBTTask_Bite::UBTTask_Bite()
{
	NodeName = "Bite";
	bCreateNodeInstance = true;
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_Bite::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AVGMonsterRunner* Runner = Cast<AVGMonsterRunner>(AIController->GetPawn());
	if (!Runner) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = Runner->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;

	AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Bite::OnMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Bite::OnMontageEnded);

	CurrentMontage = Runner->GetRandomAttackMontage();
	Runner->PlayAnimMontage(CurrentMontage);

	return EBTNodeResult::InProgress;
}

void UBTTask_Bite::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AVGMonsterRunner* Runner = Cast<AVGMonsterRunner>(OwnerComp.GetAIOwner()->GetPawn());
	if (Runner)
	{
		UAnimInstance* AnimInstance = Runner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Bite::OnMontageEnded);
		}
	}

	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

EBTNodeResult::Type UBTTask_Bite::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterRunner* Runner = Cast<AVGMonsterRunner>(OwnerComp.GetAIOwner()->GetPawn());
	if (Runner)
	{
		UAnimInstance* AnimInstance = Runner->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Bite::OnMontageEnded);
		}
		Runner->StopAnimMontage(CurrentMontage);
	}
	CachedOwnerComp = nullptr;
	return EBTNodeResult::Aborted;
}

void UBTTask_Bite::OnMontageEnded(UAnimMontage* AnimAttack, bool bInterrupted)
{
	if (bInterrupted) return;

	if (AnimAttack != CurrentMontage)
	{
		return;
	}

	if (CachedOwnerComp)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		CachedOwnerComp = nullptr;
	}
}