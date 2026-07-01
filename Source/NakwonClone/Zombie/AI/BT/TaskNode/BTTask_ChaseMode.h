// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ChaseMode.generated.h"


UCLASS()
class NAKWONCLONE_API UBTTask_ChaseMode : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ChaseMode();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// 이동 모드에서 적용할 이동 속도
	UPROPERTY(EditAnywhere, Category = "Move")
	float MoveSpeed = 400.f;
};
