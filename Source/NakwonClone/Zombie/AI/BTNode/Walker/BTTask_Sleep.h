// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Sleep.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API UBTTask_Sleep : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Sleep();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, 
		uint8* NodeMemory) override;
	
	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp, 
		uint8* NodeMemory) override;
};
