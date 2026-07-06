#include "BTTask_RequestAttackSlot.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_RequestAttackSlot::UBTTask_RequestAttackSlot()
{
	NodeName = "Request Attack Slot";
}

EBTNodeResult::Type UBTTask_RequestAttackSlot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;
	
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AICon->GetPawn());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Monster || !BB) return EBTNodeResult::Failed;
	
	const float Distance = BB->GetValueAsFloat(AVGMonsterAIControllerBase::DistanceKey);
	if (Distance > SlotRequestDistance)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target) return EBTNodeResult::Failed;

	FVector SlotLocation;
	if (!Monster->ReserveAttackSlot(Target, SlotLocation))
	{
		return EBTNodeResult::Failed;   // 슬롯 다 찼음 → Selector가 대기 브랜치로 폴백
	}

	BB->SetValueAsVector(AVGMonsterAIControllerBase::SlotLocationKey, SlotLocation);
	return EBTNodeResult::Succeeded;
}