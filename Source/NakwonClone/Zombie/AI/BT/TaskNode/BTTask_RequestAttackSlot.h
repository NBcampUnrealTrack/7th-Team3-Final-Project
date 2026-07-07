#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RequestAttackSlot.generated.h"

UCLASS()
class NAKWONCLONE_API UBTTask_RequestAttackSlot : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_RequestAttackSlot();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Detection")
	float SlotRequestDistance = 200.f;
};