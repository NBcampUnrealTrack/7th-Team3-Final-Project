// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ApplySpeedMode.generated.h"

class UGameplayEffect;

UCLASS()
class NAKWONCLONE_API UBTTask_ApplySpeedMode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ApplySpeedMode();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// 이 노드가 적용할 속도 GE (Walk / Chase / Run 등을 노드마다 다르게 지정)
	UPROPERTY(EditAnywhere, Category = "Speed")
	TSubclassOf<UGameplayEffect> SpeedEffectClass;

	// 커브테이블 레벨 (기본 1)
	UPROPERTY(EditAnywhere, Category = "Speed")
	int32 EffectLevel = 1;
};