// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_ApplySpeedMode.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_ApplySpeedMode::UBTTask_ApplySpeedMode()
{
	NodeName = "Apply Speed Mode";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ApplySpeedMode::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AIController->GetPawn());
	if (!Monster || !SpeedEffectClass) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (!ASC) return EBTNodeResult::Failed;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	ASC->ApplyGameplayEffectToSelf(SpeedEffectClass.GetDefaultObject(), EffectLevel, Context);

	return EBTNodeResult::Succeeded;
}