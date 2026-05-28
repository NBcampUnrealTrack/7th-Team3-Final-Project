// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Stop.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API UBTTask_Stop : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Stop();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

	// 정지 후 대기 시간
	UPROPERTY(EditAnywhere, Category = "Stop")
	float StopDuration = 2.f;
};
