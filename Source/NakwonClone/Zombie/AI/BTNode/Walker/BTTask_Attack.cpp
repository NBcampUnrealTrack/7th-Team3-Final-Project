// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/Walker/BTTask_Attack.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
	bCreateNodeInstance = true;
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] ExecuteTask 호출됨"));
	
	// 이 BT를 실행 중인 AI 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	// AI 컨트롤러가 빙의한 폰(몬스터)을 Walker로 캐스팅하기
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(AIController->GetPawn());
	if (!Walker) return EBTNodeResult::Failed;
	
	// 몬스터의 애니메이션 인스턴스 가져오기
	UAnimInstance* AnimInstance = Walker->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;
	
	// OwnerComp 저장하기
	CachedOwnerComp = &OwnerComp;
	
	// 몽타주가 끝나면 OnMontageEnded를 호출하도록 드록
	
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Attack::OnMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Attack::OnMontageEnded);
	
	// 공격 몽타주 재생
	CurrentMontage = Walker->GetRandomAttackMontage();
	Walker->PlayAnimMontage(CurrentMontage);
	
	// "아직 진행 중" 반환 -> BT 대기
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	// 테스크가 어떤 이유로든 끝나면 콜백 하제 + 포인터 초기화
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (Walker)
	{
		UAnimInstance* AnimInstance = Walker->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Attack::OnMontageEnded);
		}
	}
	
	// UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] OnTaskFinished 호출됨"));
	
	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (Walker)
	{
		UAnimInstance* AnimInstance = Walker->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Attack::OnMontageEnded);
		}
		Walker->StopAnimMontage(CurrentMontage);
	}
	CachedOwnerComp = nullptr;
	return EBTNodeResult::Aborted;
}

void UBTTask_Attack::OnMontageEnded(UAnimMontage* AnimAttack, bool bInterrupted)
{
	/* UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] OnMontageEnded - Montage: %s, Current: %s"),
		AnimAttack ? *AnimAttack->GetName() : TEXT("nullptr"),
		CurrentMontage ? *CurrentMontage->GetName() : TEXT("nullptr")); */
	
	// UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] bInterrupted: %s"), bInterrupted ? TEXT("true") : TEXT("false"));
	
	if (bInterrupted) return;	
	
	if (AnimAttack != CurrentMontage)
	{
		// UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] AnimAttack != CurrentMontage"));
		return;
	}
	
	if (CachedOwnerComp)
	{
		// UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] FinishLatentTask 호출 전"));
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		// UE_LOG(LogMonster, Warning, TEXT("[BTTask_Attack] FinishLatentTask 호출 후"));
		CachedOwnerComp = nullptr;
	}
}
