// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckAttackRange.generated.h"

UCLASS()
class NAKWONCLONE_API UBTService_CheckAttackRange : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckAttackRange();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;
	
private:
	// 공격 범위
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 90.f;
};
