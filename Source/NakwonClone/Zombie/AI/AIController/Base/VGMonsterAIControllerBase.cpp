// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "Perception/AIPerceptionSystem.h"

#pragma region 블랙보드 키 이름 정의
const FName AVGMonsterAIControllerBase::PatrolLocationKey = "PatrolLocation";
const FName AVGMonsterAIControllerBase::PatrolIndexKey    = "PatrolIndex";
const FName AVGMonsterAIControllerBase::TargetActorKey    = "TargetActor";
const FName AVGMonsterAIControllerBase::HeardLocationKey  = "HeardLocation";
const FName AVGMonsterAIControllerBase::IsDeadKey         = "IsDead";
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
	HearingConfig->HearingRange = 800.f;				// 청각 반경
	HearingConfig->SetMaxAge(3.f);						// 감지 정보 유지 시간 (청각)
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

		UE_LOG(LogTemp, Warning, TEXT("[AIController] Perception 콜백 바인딩 완료"));
	}
}

void AVGMonsterAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	UE_LOG(LogTemp, Warning, TEXT("[AIController] OnPossess 호출됨"));

	if (BehaviorTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AIController] BT 실행 시도"));
		UBlackboardComponent* BlackboardComp = Blackboard;
		if (UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp))
		{
			RunBehaviorTree(BehaviorTree);
			UE_LOG(LogTemp, Warning, TEXT("[AIController] BT 실행 완료"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[AIController] UseBlackboard 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[AIController] BehaviorTree가 null"));
	}
}

void AVGMonsterAIControllerBase::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Blackboard || !Actor) return;

	// 플레이어 태그 확인
	// if (!Actor->ActorHasTag("Player")) return;

#pragma region 시각 감지 처리
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// 시각 감지 성공 → TargetActor 등록
			UE_LOG(LogTemp, Warning, TEXT("[AIPerception] 시각 감지: %s"), *Actor->GetName());
			Blackboard->SetValueAsObject(TargetActorKey, Actor);
			
			AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(GetPawn());
			if (Walker) Walker->SetMonsterState(EMonsterState::Chase);
		}
		else
		{
			// 시각 감지 해제 → TargetActor 초기화
			UE_LOG(LogTemp, Warning, TEXT("[AIPerception] 시각 감지 해제: %s"), *Actor->GetName());
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
			UE_LOG(LogTemp, Warning, TEXT("[AIPerception] 청각 감지 위치: %s"), *Stimulus.StimulusLocation.ToString());
			Blackboard->SetValueAsVector(HeardLocationKey, Stimulus.StimulusLocation);
		}
	}
#pragma endregion
}

