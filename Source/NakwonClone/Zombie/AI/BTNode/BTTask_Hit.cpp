// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Hit.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTTask_Hit::UBTTask_Hit()
{
	NodeName = TEXT("Hit");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Hit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AIController->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;

	UE_LOG(LogMonster, Warning, TEXT("[BTTask_Hit] ExecuteTask 호출됨"));
	
	CurrentMontage = Monster->GetRandomHitMontage();
	Monster->PlayAnimMontage(CurrentMontage);
	CachedOwnerComp = &OwnerComp;
	
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Hit::OnMontageEnded);
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
}

void UBTTask_Hit::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentMontage) return;
	if (!CachedOwnerComp) return;
	
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	CachedOwnerComp = nullptr;
}
