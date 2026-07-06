// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateToFaceTarget.generated.h"

UCLASS()
class NAKWONCLONE_API UBTTask_RotateToFaceTarget : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_RotateToFaceTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float Precision = 5.f;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	float RotationSpeed = 720.f; // deg/sec
};