// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Dead.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Animation/AnimInstance.h"

UBTTask_Dead::UBTTask_Dead()
{
	NodeName = "Dead";
	
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_Dead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	CurrentMontage = Monster->GetSelectedDeadMontage();
	Monster->PlayAnimMontage(CurrentMontage);
	CachedOwnerComp = &OwnerComp;
	
	UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Dead::OnMontageEnded);
		AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Dead::OnMontageEnded);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Dead::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster)
	{
		UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Dead::OnMontageEnded);
		}
	}
	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Dead::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentMontage)
	{
		return;
	}
	if (!CachedOwnerComp)
	{
		return;
	}
	
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(CachedOwnerComp->GetAIOwner()->GetPawn());
	if (Monster)
	{
		Monster->HandleDead();
	}
	
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	CachedOwnerComp->StopTree(EBTStopMode::Safe);
	CachedOwnerComp = nullptr;
}
