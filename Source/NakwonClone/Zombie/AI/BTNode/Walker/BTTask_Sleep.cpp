// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Sleep.h"

#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Sleep::UBTTask_Sleep()
{
	NodeName = "Sleep";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Sleep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Monster = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	Monster->PlayAnimMontage(Monster->GetSleepMontage());

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_Sleep::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Monster = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Aborted;

	UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Aborted;

	Monster->StopAnimMontage(Monster->GetSleepMontage());
    
	CurrentMontage = Monster->GetSelectedWakeUpMontage();
	Monster->PlayAnimMontage(CurrentMontage);
    
	CachedOwnerComp = &OwnerComp;
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Sleep::OnWakeUpMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Sleep::OnWakeUpMontageEnded);

	return EBTNodeResult::InProgress;
}

void UBTTask_Sleep::OnWakeUpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentMontage) return;
	if (!CachedOwnerComp) return;

	UAnimInstance* AnimInstance = Cast<AVGMonsterWalker>(CachedOwnerComp->GetAIOwner()->GetPawn())
		->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Sleep::OnWakeUpMontageEnded);
	}

	FinishLatentAbort(*CachedOwnerComp);
	CachedOwnerComp = nullptr;
}
