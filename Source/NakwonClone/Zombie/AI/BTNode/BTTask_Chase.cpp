// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Chase.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "GameplayEffect.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Chase::UBTTask_Chase()
{
	NodeName = "Chase";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Walker) return EBTNodeResult::Failed;

	Walker->PlayAnimMontage(Walker->GetSelectedChaseMontage());
	
	UAbilitySystemComponent* ASC = Walker->GetAbilitySystemComponent();
	if (ASC && Walker->ChaseSpeedEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		ASC->ApplyGameplayEffectToSelf(Walker->ChaseSpeedEffectClass.GetDefaultObject(), Walker->GetSelectedChaseLevel(), Context);
	}
	
	return EBTNodeResult::Succeeded;
}