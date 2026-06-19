// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Move.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "GameplayEffect.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTTask_Move::UBTTask_Move()
{
	NodeName = "Move";
}

EBTNodeResult::Type UBTTask_Move::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (!Monster) return EBTNodeResult::Failed;

	Monster->PlayAnimMontage(Monster->GetSelectedMoveMontage());
	
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (ASC && Monster->SpeedEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		ASC->ApplyGameplayEffectToSelf(Monster->SpeedEffectClass.GetDefaultObject(), 2.f, Context);
	}
	
	return EBTNodeResult::Succeeded;
}
