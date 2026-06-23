// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume.h"

#include "AIController.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/SpawnBox/MonsterSpawnRow.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);
	
	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SpawnBox->SetupAttachment(Scene);
	
	SpawnInterval = 5.0f;
	SpawnInit = 10.0f;
	StopSpawnTime = 540.0f;
	MaxSpawnCount = 9;
}


void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < SpawnInit; i++)
	{
		SpawnRandomMonster();
	}
}

void ASpawnVolume::UpdateWave(float CurrentTime)
{
	// 종류 시간 체크
	if (CurrentTime >= StopSpawnTime)
	{
		return;
	}
	
	// 현재 시간 / 15초 > 1 / 15 = 0.xxx이므로 0. 16 / 15 = 1.xxx이므로 1로 변환
	// 이걸로 웨이브 주기 판단함
	int32 CurrentIntervalIndex = FMath::FloorToInt(CurrentTime / SpawnInterval);
	
	if (CurrentIntervalIndex > LastSpawnCheckTime)
	{
		// 60초 이전까지는 1마리, 그 이후로 1분 주기마다 생성시 스폰되는 몬스터 1마리씩 추가
		int32 Wave = FMath::FloorToInt(CurrentTime / 60.0f);
		int32 SpawnCount = FMath::Clamp(Wave+1, 0, MaxSpawnCount);
		
		SpawnMonsters(SpawnCount);
		
		LastSpawnCheckTime = CurrentIntervalIndex;
	}
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
	for (int i = 0; i < Count; i++)
	{
		SpawnRandomMonster();
	}
}

void ASpawnVolume::SpawnMonster(TSubclassOf<AActor> MonsterClass)
{
	if (!MonsterClass || !GetWorld())
	{
		return;
	}
	
	FVector SpawnLocation = SpawnBox->GetComponentLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	
	AActor* NewActor = GetWorld()->SpawnActor<AActor>(
		MonsterClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
		);
	
	if (NewActor)
	{
		APawn* MonsterPawn = Cast<APawn>(NewActor);
		if (MonsterPawn)
		{
			AAIController* AIC = Cast<AAIController>(MonsterPawn->GetController());
			if (AIC && AIC->GetBlackboardComponent())
			{
				// 상대 좌표(위젯)를 월드 좌표로 변환하여 전달
				FVector WorldTarget = GetActorLocation() + TargetEndingLocation;
                
				// 블랙보드 키 이름 'EndingLocation'은 BT의 이름과 반드시 일치해야 함
				AIC->GetBlackboardComponent()->SetValueAsVector(TEXT("EndingLocation"), WorldTarget);
			}
		}
	}}


