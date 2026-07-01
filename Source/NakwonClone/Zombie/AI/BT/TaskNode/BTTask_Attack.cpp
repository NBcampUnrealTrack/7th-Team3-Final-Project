// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

    // base로 캐스팅 (Walker/Witch/Tank 다 됨)
    AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(AICon->GetPawn());
    if (!Monster) return EBTNodeResult::Failed;

    CachedMonster = Monster;
    CachedOwnerComp = &OwnerComp;

    // 몽타주 끝나면 이 태스크 완료
    Monster->OnAttackFinished.BindLambda([this]()
        {
            if (CachedOwnerComp)
            {
                UBehaviorTreeComponent* Comp = CachedOwnerComp;
                CachedOwnerComp = nullptr;
                FinishLatentTask(*Comp, EBTNodeResult::Succeeded);
            }
        });

    // 몽타주 직접 재생 안 함 — 캐릭터한테 "공격해"만
    Monster->StartAttack();

    return EBTNodeResult::InProgress;   // 몽타주 끝날 때까지 대기
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (CachedMonster)
    {
        CachedMonster->OnAttackFinished.Unbind();
    }
    CachedOwnerComp = nullptr;
    return EBTNodeResult::Aborted;
}