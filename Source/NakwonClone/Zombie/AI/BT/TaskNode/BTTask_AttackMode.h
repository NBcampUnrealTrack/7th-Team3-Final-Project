// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackMode.generated.h"

/**
 * 
 */
UCLASS()
class NAKWONCLONE_API UBTTask_AttackMode : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_AttackMode();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 공격 사거리
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 90.f;
};
