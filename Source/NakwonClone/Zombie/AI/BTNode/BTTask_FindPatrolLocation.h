// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPatrolLocation.generated.h"


UCLASS()
class NAKWONCLONE_API UBTTask_FindPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindPatrolLocation();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	// 랜덤 좌표 탐색 반경
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float SearchRadius = 500.0f;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Patrol")
	int32 MaxPatrolCount = 3;
};
