// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Stop.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Stop::UBTTask_Stop()
{
	NodeName = "Stop";
}

EBTNodeResult::Type UBTTask_Stop::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 이동 정지
	OwnerComp.GetAIOwner()->StopMovement();
	
	// StopDuration 이후 InProgress (InProgress : 다음 틱에서 완료 처리)
	return EBTNodeResult::InProgress;
}

void UBTTask_Stop::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 대기 시간이 지나면 HeardLocation 초기화 후 완료
	float& Elapsed = *reinterpret_cast<float*>(NodeMemory);
	Elapsed += DeltaSeconds;
	
	if (Elapsed > StopDuration)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(
			AVGMonsterAIControllerBase::HeardLocationKey,
			FVector::ZeroVector);
		
		AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
		if (Walker) Walker->SetMonsterState(EMonsterState::Stop);
		
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

uint16 UBTTask_Stop::GetInstanceMemorySize() const
{
	// Elapsed 저장용
	return sizeof(float);
}