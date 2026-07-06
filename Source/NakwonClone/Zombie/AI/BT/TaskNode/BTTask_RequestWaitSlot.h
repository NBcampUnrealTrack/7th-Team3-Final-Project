// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RequestWaitSlot.generated.h"

UCLASS()
class NAKWONCLONE_API UBTTask_RequestWaitSlot : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_RequestWaitSlot();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
