// Fill out your copyright notice in the Description page of Project Settings.


#include "NakwonClone/Zombie/AI/BTNode/BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 컨트롤러 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	// 몬스터 가져오기
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(AIController->GetPawn());
	if (!Walker) return EBTNodeResult::Failed;
	// 몬스터 상태 변경
	Walker->SetMonsterState(EMonsterState::Attack);
	
	// 블랙보드에서 타겟 액터 가져오기
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return EBTNodeResult::Failed;
	
	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(
	AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target) return EBTNodeResult::Failed;
	
	// 타겟의 ASC 가져오기
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC) return EBTNodeResult::Failed;
	
	// 몬스터의 ASC 가져오기 (GE 적용 주체)
	UAbilitySystemComponent* MonsterASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Walker);
	if (!MonsterASC) return EBTNodeResult::Failed;
	
	// GE_Attack 적용
	if (AttackEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = MonsterASC->MakeEffectContext();
		EffectContext.AddSourceObject(Walker);
		
		FGameplayEffectSpecHandle SpecHandle = MonsterASC->MakeOutgoingSpec(
			AttackEffectClass, 2.f, EffectContext);
		
		if (SpecHandle.IsValid())
		{
			MonsterASC->ApplyGameplayEffectSpecToTarget(
				*SpecHandle.Data.Get(), TargetASC);
			
			UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] GE_Attack 적용 완료"));
		}
	}
	
	return EBTNodeResult::Succeeded;
}
