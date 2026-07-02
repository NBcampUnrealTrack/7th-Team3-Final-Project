#include "BTTask_StateMode.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_StateMode::UBTTask_StateMode()
{
	NodeName = "State Mode";
}

EBTNodeResult::Type UBTTask_StateMode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterAIControllerBase* AICon = Cast<AVGMonsterAIControllerBase>(OwnerComp.GetAIOwner());
	if (!AICon) return EBTNodeResult::Failed;

	if (ACharacter* SelfChar = Cast<ACharacter>(AICon->GetPawn()))
	{
		if (AVGMonsterCharacterBase* Mon = Cast<AVGMonsterCharacterBase>(SelfChar))
		{
			if (SelfChar->GetCharacterMovement())
				SelfChar->GetCharacterMovement()->MaxWalkSpeed = Mon->GetPatrolSpeed();
		}
	}

	return AICon->FindPatrolLocation() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}