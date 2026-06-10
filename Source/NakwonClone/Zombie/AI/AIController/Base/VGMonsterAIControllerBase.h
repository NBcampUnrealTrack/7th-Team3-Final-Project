// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMonster, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAIPc, Log, All);

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "VGMonsterAIControllerBase.generated.h"

UCLASS()
class NAKWONCLONE_API AVGMonsterAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AVGMonsterAIControllerBase();
	
#pragma region 블랙보드 키 이름
	static const FName PatrolLocationKey;
	static const FName PatrolIndexKey;
	static const FName TargetActorKey;
	static const FName HeardLocationKey;
	static const FName IsDeadKey;
#pragma endregion

#pragma region Behavior Tree & Blackboard
	UPROPERTY(EditAnywhere, Category = "AI|BehaviorTree")
	UBehaviorTree* BehaviorTree;
#pragma endregion
	
#pragma region AIPerception
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	UAIPerceptionComponent* MonsterPerceptionComponent;

	UPROPERTY(EditAnywhere, Category = "AI|Perception")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditAnywhere, Category = "AI|Perception")
	UAISenseConfig_Hearing* HearingConfig;
#pragma endregion

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	// 감지 콜백
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};