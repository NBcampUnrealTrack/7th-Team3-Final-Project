// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

UCLASS()
class NAKWONCLONE_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Attack();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp, 
		uint8* NodeMemory, 
		EBTNodeResult::Type TaskResult) override;
	
private:
	// 몽타주 종료 시 FinishLatentTask 호출
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentMontage;
	
	// 몽타주 종료 시 자동으로 실행되는 델리게이트
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* AnimAttack, bool bInterrupted);
};