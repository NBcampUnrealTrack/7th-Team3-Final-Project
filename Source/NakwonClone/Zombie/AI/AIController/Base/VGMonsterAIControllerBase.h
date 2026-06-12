// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "VGMonsterAIControllerBase.generated.h"

// 전방 선언
class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

DECLARE_LOG_CATEGORY_EXTERN(LogMonster, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogAIPc, Log, All);

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
	static const FName IsAttackKey;
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