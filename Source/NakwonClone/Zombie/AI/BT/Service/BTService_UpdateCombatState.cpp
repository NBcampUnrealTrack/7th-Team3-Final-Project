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
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UBTService_UpdateCombatState::UBTService_UpdateCombatState()
{
	NodeName = "UpdateCombatState";
	
	Interval = 0.1f;
	RandomDeviation = 0.05f;
	
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
	if (!SelfPawn || !BB) return; // perception 제거

	AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(SelfPawn);

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
		if (Monster) Monster->ReleaseAttackSlot();
		return;
	}

	// ── 캡슐 반지름 빼서 "표면 사이 거리"로 보정 ──
	float SelfRadius = 0.f;
	if (ACharacter* SelfChar = Cast<ACharacter>(SelfPawn))
	{
		if (UCapsuleComponent* Cap = SelfChar->GetCapsuleComponent())
			SelfRadius = Cap->GetScaledCapsuleRadius();
	}
	float TargetRadius = 0.f;
	if (ACharacter* TargetChar = Cast<ACharacter>(NearestPlayer))
	{
		if (UCapsuleComponent* Cap = TargetChar->GetCapsuleComponent())
			TargetRadius = Cap->GetScaledCapsuleRadius();
	}
	NearestDist = FMath::Max(0.f, NearestDist - SelfRadius - TargetRadius);

	// ── 2) 거리 기반 전투 판별 ──
	const bool bIsCombat = (NearestDist <= DetectRadius);
	BB->SetValueAsBool(AVGMonsterAIControllerBase::BIsCombatKey, bIsCombat);
	BB->SetValueAsFloat(AVGMonsterAIControllerBase::DistanceKey, NearestDist);

	if (bIsCombat)
	{
		BB->SetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey, NearestPlayer);

		if (Monster)
		{
			FVector SlotLoc;
			bool bReserved = false;

			// ── 사거리 안 → Attack Slot 우선 시도 ──
			if (NearestDist <= SlotReleaseDistance)
			{
				bReserved = Monster->ReserveAttackSlot(NearestPlayer, SlotLoc);
			}

			// ── 사거리 밖이거나 Attack Slot이 꽉 찼음 → Wait Slot ──
			if (!bReserved)
			{
				bReserved = Monster->ReserveWaitSlot(NearestPlayer, SlotLoc);
			}

			// ── 매 틱 최신 슬롯 위치를 BB에 갱신 (MoveTo가 Observed Blackboard Value로 실시간 추적) ──
			if (bReserved)
			{
				BB->SetValueAsVector(AVGMonsterAIControllerBase::SlotLocationKey, SlotLoc);
			}
		}
	}
	else
	{
		BB->ClearValue(AVGMonsterAIControllerBase::TargetActorKey);
		if (Monster) Monster->ReleaseAttackSlot();
	}
}