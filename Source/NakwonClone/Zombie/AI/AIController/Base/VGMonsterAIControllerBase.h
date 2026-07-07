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

#pragma region 코어/라이프사이클
public:
	AVGMonsterAIControllerBase();
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
#pragma endregion

#pragma region 블랙보드 키 이름
public:
	static const FName TargetActorKey;       // 시야에 잡힌 플레이어 (Object)
	static const FName BIsCombatKey;         // 전투 판별 결과 (Bool)
	static const FName BIsAttackKey;         // 공격 여부 판별 결과 (Bool)
	static const FName DistanceKey;          // 좀비-타겟 거리 (Float)
	static const FName HeardLocationKey;     // 들린 소리 위치 (Vector)
	static const FName PatrolLocationKey;    // 순찰 목적지 좌표 (Vector)
	static const FName SlotLocationKey;      // 예약된 공격 슬롯 좌표 (Vector)
#pragma endregion
	
#pragma region Patrol
public:
	bool FindPatrolLocation();
	
protected:
	UPROPERTY(EditAnywhere, Category = "AI|Patrol")
	float SearchRadius = 500.f;
#pragma endregion
	
#pragma region Behavior Tree & Blackboard
protected:
	UPROPERTY(EditAnywhere, Category = "AI|BehaviorTree")
	UBehaviorTree* BehaviorTree;
#pragma endregion
	
#pragma region AIPerception
protected:
	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	UAIPerceptionComponent* MonsterPerceptionComponent;

	UPROPERTY(EditAnywhere, Category = "AI|Perception")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditAnywhere, Category = "AI|Perception")
	UAISenseConfig_Hearing* HearingConfig;
	
private:
	// 감지 콜백 — [BT 리팩터] 새 Service로 대체, 정의부 주석 처리에 맞춰 선언도 주석
	//UFUNCTION()
	//void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 수면 (AI)
public:
	bool CanSleep() const { return bCanSleep; }
protected:
	UPROPERTY(EditAnywhere, Category = "AI|Perception")
	bool bCanSleep = false;
#pragma endregion
};