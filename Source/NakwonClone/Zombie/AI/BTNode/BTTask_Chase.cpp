// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Chase.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "GameplayEffect.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_Chase::UBTTask_Chase()
{
	NodeName = "Chase";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;
	
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (ASC && Monster->ChaseSpeedEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		ASC->ApplyGameplayEffectToSelf(
			Monster->ChaseSpeedEffectClass.GetDefaultObject(),
			Monster->GetSelectedChaseLevel(), Context);
	}
	
	return EBTNodeResult::Succeeded;
}