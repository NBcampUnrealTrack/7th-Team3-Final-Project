// 헌호수정 - 심플 추격 서비스: 가장 가까운 플레이어를 TargetActor로, 거리를 Distance로 세팅
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ChaseTarget.generated.h"

UCLASS()
class NAKWONCLONE_API UBTService_ChaseTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ChaseTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	//헌호수정 - 이 거리보다 멀면 물리 뼈 끔(성능↑), 가까우면 켬(부위 피격/헤드샷 정상). 에디터 조절 가능
	UPROPERTY(EditAnywhere, Category = "LOD")
	float PhysicsBoneLODDistance = 1000.f;
};
