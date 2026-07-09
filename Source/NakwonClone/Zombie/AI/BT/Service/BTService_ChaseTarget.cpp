// 헌호수정 - 심플 추격 서비스 구현
#include "BTService_ChaseTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h" //헌호수정 - 물리 뼈 LOD
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTService_ChaseTarget::UBTService_ChaseTarget()
{
	NodeName = "Chase Target (Simple)";

	Interval = 0.2f;          // 0.2초마다 갱신 (성능: 100마리에도 가벼움)
	RandomDeviation = 0.05f;  // 좀비들이 동시에 안 몰리게 살짝 분산

	bNotifyTick = true;
	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = false;
}

void UBTService_ChaseTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	APawn* SelfPawn = AICon->GetPawn();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!SelfPawn || !BB) return;

	const FVector SelfLoc = SelfPawn->GetActorLocation();

	// ── 가장 가까운 플레이어 찾기 (거리 제한 없음 → 시작하자마자 추격) ──
	APawn* NearestPlayer = nullptr;
	float NearestDist = TNumericLimits<float>::Max();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* P = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (!P) continue;

		const float D = FVector::Dist(SelfLoc, P->GetActorLocation());
		if (D < NearestDist)
		{
			NearestDist = D;
			NearestPlayer = P;
		}
	}

	// ── 플레이어 없으면 타겟 클리어 ──
	if (!NearestPlayer)
	{
		BB->ClearValue(AVGMonsterAIControllerBase::TargetActorKey);
		return;
	}

	// ── 캡슐 반지름 빼서 "표면 사이 거리"로 보정 (공격 범위 판정 정확하게) ──
	float SelfRadius = 0.f;
	if (const ACharacter* SelfChar = Cast<ACharacter>(SelfPawn))
	{
		if (const UCapsuleComponent* Cap = SelfChar->GetCapsuleComponent())
			SelfRadius = Cap->GetScaledCapsuleRadius();
	}
	float TargetRadius = 0.f;
	if (const ACharacter* TargetChar = Cast<ACharacter>(NearestPlayer))
	{
		if (const UCapsuleComponent* Cap = TargetChar->GetCapsuleComponent())
			TargetRadius = Cap->GetScaledCapsuleRadius();
	}
	const float SurfaceDist = FMath::Max(0.f, NearestDist - SelfRadius - TargetRadius);

	// ── 블랙보드에 타겟 + 거리 세팅 ──
	BB->SetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey, NearestPlayer);
	BB->SetValueAsFloat(AVGMonsterAIControllerBase::DistanceKey, SurfaceDist);

	//헌호수정 - 거리 기반 물리 뼈 LOD: 멀면 끔(성능↑), 가까우면 켬(부위별 피격/헤드샷 유지)
	if (ACharacter* SelfCharForLOD = Cast<ACharacter>(SelfPawn))
	{
		if (USkeletalMeshComponent* Mesh = SelfCharForLOD->GetMesh())
		{
			const bool bNear = (SurfaceDist <= PhysicsBoneLODDistance);
			// 가까움 → 애니에 맞춰 물리 뼈 갱신(정밀 피격) / 멂 → 갱신 스킵(비용↓)
			const EKinematicBonesUpdateToPhysics::Type Desired =
				bNear ? EKinematicBonesUpdateToPhysics::SkipSimulatingBones
				      : EKinematicBonesUpdateToPhysics::SkipAllBones;

			if (Mesh->KinematicBonesUpdateType != Desired)
			{
				Mesh->KinematicBonesUpdateType = Desired;
			}
		}
	}
}
