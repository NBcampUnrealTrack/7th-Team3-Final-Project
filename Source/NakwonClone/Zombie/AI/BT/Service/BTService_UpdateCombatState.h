// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateCombatState.generated.h"


UCLASS()
class NAKWONCLONE_API UBTService_UpdateCombatState : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateCombatState();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Detection")
	float DetectRadius = 1500.f;
};
