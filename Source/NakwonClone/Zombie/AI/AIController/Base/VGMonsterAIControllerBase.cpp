// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Common/NCGameplayTags.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

DEFINE_LOG_CATEGORY(LogMonster);
DEFINE_LOG_CATEGORY(LogAIPc);

#pragma region 블랙보드 키 이름 정의
const FName AVGMonsterAIControllerBase::PatrolLocationKey = "PatrolLocation";
const FName AVGMonsterAIControllerBase::TargetActorKey    = "TargetActor";
const FName AVGMonsterAIControllerBase::HeardLocationKey  = "HeardLocation";
const FName AVGMonsterAIControllerBase::IsDeadKey         = "bIsDead";
const FName AVGMonsterAIControllerBase::IsAttackKey       = "bIsAttack";
const FName AVGMonsterAIControllerBase::IsHitKey          = "bIsHit";
const FName AVGMonsterAIControllerBase::IsAwakeKey        = "bIsAwake";
const FName AVGMonsterAIControllerBase::IsWanderingKey    = "bIsWandering";
const FName AVGMonsterAIControllerBase::PatrolCountKey    = "PatrolCount";
#pragma endregion

AVGMonsterAIControllerBase::AVGMonsterAIControllerBase()
{
	PrimaryActorTick.bCanEverTick = false;

#pragma region AIPerception 초기화
	// 컴포넌트 생성
	MonsterPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent((*MonsterPerceptionComponent));
	
	// 시각 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1000.f;						 // 시야 반경
	SightConfig->LoseSightRadius = 1200.f;					 // 시야 해제 반경
	SightConfig->PeripheralVisionAngleDegrees = 60.f;		 // 시야각 (좌우 합산 120도)
	SightConfig->SetMaxAge(5.f);							 // 감지 정보 유지 시간 (시각)
	
	// 플레이어 태그를 가진 액터만 감지
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	// 청각 설정
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 5000.f;				// 청각 반경
	HearingConfig->SetMaxAge(60.f);						// 감지 정보 유지 시간 (청각)
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

	// Perception 컴포넌트에 감각 등록
	MonsterPerceptionComponent->ConfigureSense(*SightConfig);
	MonsterPerceptionComponent->ConfigureSense(*HearingConfig);
	MonsterPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
#pragma endregion
}

void AVGMonsterAIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (MonsterPerceptionComponent)
	{
		// 혹시 이미 등록돼 있으면 먼저 지우기
		MonsterPerceptionComponent->OnTargetPerceptionUpdated.RemoveDynamic(
			this,
			&AVGMonsterAIControllerBase::OnPerceptionUpdated);

		// 그 다음 새로 등록
		MonsterPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&AVGMonsterAIControllerBase::OnPerceptionUpdated);

		UE_LOG(LogMonster, Warning, TEXT("[AIController] Perception 콜백 바인딩 완료"));
	}
}

void AVGMonsterAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	

	if (BehaviorTree)
	{
		UE_LOG(LogMonster, Warning, TEXT("[AIController] BT 실행 시도"));
		UBlackboardComponent* BlackboardComp = Blackboard;
		if (UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp))
		{
			RunBehaviorTree(BehaviorTree);
			UE_LOG(LogMonster, Warning, TEXT("[AIController] BT 실행 완료"));
		}
		else
		{
			UE_LOG(LogMonster, Error, TEXT("[AIController] UseBlackboard 실패"));
		}
	}
	else
	{
		UE_LOG(LogMonster, Error, TEXT("[AIController] BehaviorTree가 null"));
	}
}

void AVGMonsterAIControllerBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Blackboard || !Actor) return;

	// ASC에서 플레이어 태그 확인
	UAbilitySystemComponent* TargetASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (!TargetASC) return;
	if (!TargetASC->HasMatchingGameplayTag(NCCharacter::Player)) return;

#pragma region 시각 감지 처리
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (!Blackboard->GetValueAsBool(IsAwakeKey)) return;
		
		if (Stimulus.WasSuccessfullySensed())
		{
			// 시각 감지 성공 → TargetActor 등록
			UE_LOG(LogAIPc, Warning, TEXT("[AIPerception] 시각 감지: %s"), *Actor->GetName());
			Blackboard->SetValueAsObject(TargetActorKey, Actor);
			Blackboard->ClearValue(HeardLocationKey);
		}
		else
		{
			// 시각 감지 해제 → TargetActor 초기화
			UE_LOG(LogAIPc, Warning, TEXT("[AIPerception] 시각 감지 해제: %s"), *Actor->GetName());
			Blackboard->ClearValue(TargetActorKey);
		}
	}
#pragma endregion

#pragma region 청각 감지 처리
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 청각 감지 성공 → 소리 발생 위치 등록
			// UE_LOG(LogAIPc, Warning, TEXT("[AIPerception] 청각 감지 위치: %s"), *Stimulus.StimulusLocation.ToString());
			Blackboard->SetValueAsVector(HeardLocationKey, Stimulus.StimulusLocation);
    
			AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(GetPawn());
			if (Monster)
			{
				Monster->WakeUpWithDelay();
			}
		}
	}
#pragma endregion
}

