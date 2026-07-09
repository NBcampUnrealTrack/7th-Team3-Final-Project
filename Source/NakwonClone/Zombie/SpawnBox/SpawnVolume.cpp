// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"
#include "AIController.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TimerManager.h" //헌호수정
#include "EngineUtils.h" //헌호수정 - TActorIterator (좀비 수 세기)
#include "NakwonClone/Zombie/SpawnBox/MonsterSpawnRow.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SpawnBox->SetupAttachment(Scene);
	
	//헌호수정 - 추천 기본값: 3초마다 스폰, 초기 10마리, 웨이브 최대 9마리
	SpawnInterval = 3.0f;
	SpawnInit = 10.0f;
	StopSpawnTime = 540.0f;
	MaxSpawnCount = 9;
}


void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		ActivateSpawner();
	}
}

//헌호수정 - 웨이브 없이 무한 스폰: 3초마다 2~3마리 (상한 MaxAliveZombies는 SpawnMonsters에서 체크)
void ASpawnVolume::UpdateWave(float CurrentTime)
{
	const int32 SpawnCount = FMath::RandRange(2, 3); // 한 번에 2~3마리 랜덤
	SpawnMonsters(SpawnCount);
}

void ASpawnVolume::SpawnRandomMonster()
{
	if (FMonsterSpawnRow* SelectedRow = GetRandomMonster())
	{
		UClass* ActualClass = SelectedRow->MonsterClass.LoadSynchronous();
		
		if (ActualClass)
		{
			SpawnMonster(ActualClass);
		}
	}
}

FMonsterSpawnRow* ASpawnVolume::GetRandomMonster() const
{
	if (!MonsterDataTable) return nullptr;
    
	TArray<FMonsterSpawnRow*> AllRows;
	MonsterDataTable->GetAllRows(TEXT(""), AllRows);
    
	if (AllRows.IsEmpty()) return nullptr;
    
	float TotalChance = 0.0f;
	for (const FMonsterSpawnRow* Row : AllRows)
	{
		if (Row) TotalChance += Row->SpawnChance;
	}

	const float RandValue = FMath::FRand() * TotalChance;
	float AccumulateChance = 0.0f;

	for (FMonsterSpawnRow* Row : AllRows)
	{
		AccumulateChance += Row->SpawnChance;
		if (RandValue <= AccumulateChance)
		{
			return Row;
		}
	}

	return AllRows.Last();
}

void ASpawnVolume::SpawnMonsters(int32 Count)
{
	//헌호수정 - 상한 체크: 살아있는 좀비가 최대치면 스폰 스킵 (성능/안정성)
	int32 Alive = CountAliveZombies();

	for (int i = 0; i < Count; i++)
	{
		if (Alive >= MaxAliveZombies)
		{
			break; // 최대치 도달 → 더 스폰 안 함
		}
		SpawnRandomMonster();
		++Alive;
	}
}

//헌호수정 - 월드의 살아있는 좀비 수 세기
int32 ASpawnVolume::CountAliveZombies() const
{
	int32 Count = 0;
	for (TActorIterator<AVGMonsterCharacterBase> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It) && !It->IsDead())
		{
			++Count;
		}
	}
	return Count;
}

//헌호수정 - 타이머가 주기적으로 호출 → 경과 시간으로 웨이브 스폰
void ASpawnVolume::TickWave()
{
	const float Elapsed = GetWorld()->GetTimeSeconds() - SpawnStartTime;
	UpdateWave(Elapsed);
}

void ASpawnVolume::SpawnMonster(TSubclassOf<AActor> MonsterClass)
{
	if (!MonsterClass || !GetWorld())
	{
		return;
	}

	const FVector BoxOrigin = SpawnBox->GetComponentLocation();
	const FVector BoxExtent = SpawnBox->GetScaledBoxExtent();

	FVector SpawnLocation = BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		0.f
	);

	FRotator SpawnRotation = FRotator::ZeroRotator;
	FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(
		MonsterClass, SpawnTransform, this, GetInstigator());

	if (!NewActor)
	{
		return;
	}

	if (AllowedMonsterTypes.Num() > 0)
	{
		if (AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(NewActor))
		{
			Monster->AllowedRandomTypes = AllowedMonsterTypes;
		}
	}

	NewActor->FinishSpawning(SpawnTransform);

	APawn* MonsterPawn = Cast<APawn>(NewActor);
	if (MonsterPawn)
	{
		AAIController* AIC = Cast<AAIController>(MonsterPawn->GetController());
		if (AIC && AIC->GetBlackboardComponent())
		{
			FVector WorldTarget = GetActorLocation() + TargetEndingLocation;
			AIC->GetBlackboardComponent()->SetValueAsVector(TEXT("EndingLocation"), WorldTarget);
		}
	}
}

void ASpawnVolume::ActivateSpawner()
{
	if (bActivated)
	{
		return; 
	}
	bActivated = true;

	//헌호수정 - 초기 스폰 (상한 체크 포함)
	SpawnMonsters(static_cast<int32>(SpawnInit));

	//헌호수정 - 주기적 스폰 타이머 시작 (SpawnInterval마다 UpdateWave 호출)
	SpawnStartTime = GetWorld()->GetTimeSeconds();
	LastSpawnCheckTime = -1.0f;
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle, this, &ASpawnVolume::TickWave, SpawnInterval, true);
}
