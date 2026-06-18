// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Attack.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
	
	CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 이 BT를 실행 중인 AI 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	// AI 컨트롤러가 빙의한 폰(몬스터)을 Walker로 캐스팅하기
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AIController->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;
	
	// 몬스터의 애니메이션 인스턴스 가져오기
	UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;
	
	// OwnerComp 저장하기
	CachedOwnerComp = &OwnerComp;
	
	// 몽타주가 끝나면 OnMontageEnded를 호출하도록 드록
	
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Attack::OnMontageEnded);
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Attack::OnMontageEnded);
	
	// 공격 몽타주 재생
	CurrentMontage = Monster->GetRandomAttackMontage();
	Monster->PlayAnimMontage(CurrentMontage);
	
	// "아직 진행 중" 반환 -> BT 대기
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	// 테스크가 어떤 이유로든 끝나면 콜백 하제 + 포인터 초기화
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (Monster)
	{
		UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Attack::OnMontageEnded);
		}
	}
	
	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Attack::OnMontageEnded(UAnimMontage* AnimAttack, bool bInterrupted)
{
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
