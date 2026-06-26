// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Dead.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Animation/AnimInstance.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "Components/CapsuleComponent.h"

UBTTask_Dead::UBTTask_Dead()
{
	NodeName = "Dead";
	bCreateNodeInstance = true;
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_Dead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	UE_LOG(LogMonster, Warning, TEXT("[BTTask_Dead] ExecuteTask 호출됨"));
	
	CurrentMontage = Monster->GetSelectedDeadMontage();
	float Duration = Monster->PlayAnimMontage(CurrentMontage);
	
	UE_LOG(LogMonster, Warning, TEXT("DeadMontage: %s, Duration: %f"), 
		CurrentMontage ? *CurrentMontage->GetName() : TEXT("nullptr"), Duration);
	
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
}

void UBTTask_Dead::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentMontage) return;
	if (bInterrupted) return;
	if (!CachedOwnerComp) return;
	
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(CachedOwnerComp->GetAIOwner()->GetPawn());

	if (AAIController* AIC = Cast<AAIController>(Monster->GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
		
		if (Monster)
		{
			Monster->OnStartRagdoll();
			Monster->SetLifeSpan(200.f);
		}
	}

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	CachedOwnerComp->StopTree(EBTStopMode::Safe);
	CachedOwnerComp = nullptr;
}

