// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdateCombatState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Common/NCGameplayTags.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTService_UpdateCombatState::UBTService_UpdateCombatState()
{
	NodeName = "UpdateCombatState";
	
	Interval = 0.2f;
	RandomDeviation = 0.1f;
	
	bNotifyTick = true;
	bNotifyBecomeRelevant = false;   // 서비스가 처음 활성화될 때 초기화x
	bNotifyCeaseRelevant = false;    // 서비스가 비활성화될 때 초기화x
}

void UBTService_UpdateCombatState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// ── 유효성 검사 ──
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;
	APawn* SelfPawn = AICon->GetPawn();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	UAIPerceptionComponent* Perception = AICon->GetPerceptionComponent();
	if (!SelfPawn || !BB || !Perception) return;

	const FVector SelfLoc = SelfPawn->GetActorLocation();

	// ── 1) 최근접 플레이어 ──
	APawn* NearestPlayer = nullptr;
	float NearestDist = TNumericLimits<float>::Max();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (!P) continue;
		const float D = FVector::Dist(SelfLoc, P->GetActorLocation());
		if (D < NearestDist) { NearestDist = D; NearestPlayer = P; }
	}
	if (!NearestPlayer)
	{
		BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsCombatKey, false);
		BB->ClearValue(AVGMonsterAIControllerBase::TargetActorKey);
		return;
	}

	// ── 2) 거리 기반 전투 판별 ──
	const bool bIsCombat = (NearestDist <= 500.f);
	BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsCombatKey, bIsCombat);
	BB->SetValueAsFloat(AVGMonsterAIControllerBase::DistanceKey, NearestDist);  // ← 항상 갱신
	// ── 2-1) 공격 거리 판별 (가까우면 공격) ──
	const bool bIsAttack = (NearestDist <= 150.f);
	BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsAttackKey, bIsAttack);

	if (bIsCombat)
	{
		BB->SetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey, NearestPlayer);
	}
	else
	{
		BB->ClearValue(AVGMonsterAIControllerBase::TargetActorKey);
	}
}
	
	/*// ── 2) 시야 판별 (거리 무관) ──
	TArray<AActor*> SeenActors;
	Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);
	const bool bCanSee = SeenActors.Contains(NearestPlayer);

	// ── 3) 청각 판별 (거리 무관) ──
	bool bCanHear = false;
	{
		TArray<AActor*> HeardActors;
		Perception->GetCurrentlyPerceivedActors(UAISense_Hearing::StaticClass(), HeardActors);
		if (HeardActors.Contains(NearestPlayer))
		{
			FActorPerceptionBlueprintInfo Info;
			Perception->GetActorsPerception(NearestPlayer, Info);
			for (const FAIStimulus& S : Info.LastSensedStimuli)
			{
				if (S.Type == UAISense::GetSenseID<UAISense_Hearing>() && S.WasSuccessfullySensed())
				{
					BB->SetValueAsVector(AVGMonsterAIControllerBase::HeardLocationKey, S.StimulusLocation);
					bCanHear = true;
					break;
				}
			}
		}
	}
	
	// ── 4) 전투 판별 = 시야 or 청각 ──
	const bool bIsCombat = (bCanSee || bCanHear);
	BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsCombatKey, bIsCombat);

	// ── 5) 키 세팅 ──
	if (bCanSee)
	{
		// 보임 → 실시간 추적 대상 확정 + 거리 저장(판단은 AttackMode가)
		BB->SetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey, NearestPlayer);
		BB->SetValueAsFloat(AVGMonsterAIControllerBase::DistanceKey, NearestDist);
	}
	else
	{
		// 소리로만 잡힘 → 액터 추적 해제 (HeardLocation으로 이동)
		BB->ClearValue(AVGMonsterAIControllerBase::TargetActorKey);
	}*/
//}