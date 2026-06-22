// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Move.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "GameplayEffect.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Move::UBTTask_Move()
{
	NodeName = "Move";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Move::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Walker) return EBTNodeResult::Failed;

	Walker->PlayAnimMontage(Walker->GetSelectedMoveMontage());
	
	UAbilitySystemComponent* ASC = Walker->GetAbilitySystemComponent();
	if (ASC && Walker->WalkSpeedEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		ASC->ApplyGameplayEffectToSelf(Walker->WalkSpeedEffectClass.GetDefaultObject(), Walker->GetSelectedMoveLevel(), Context);
	}
	
	return EBTNodeResult::Succeeded;
}
