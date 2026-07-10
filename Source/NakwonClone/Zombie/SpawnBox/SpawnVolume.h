// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterTypeData.h"
#include "SpawnVolume.generated.h"

struct FMonsterSpawnRow;
class USceneComponent;
class UBoxComponent;


UCLASS()
class NAKWONCLONE_API ASpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ASpawnVolume();
	
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void UpdateWave(float CurrentTime);

	// 에디터에서 엔딩 때, 몬스터가 이동할 최종 목적지를 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ending", meta = (MakeEditWidget = true))
	FVector TargetEndingLocation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	UBoxComponent* SpawnBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* MonsterDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnInterval;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float StopSpawnTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 MaxSpawnCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnInit;

	// 시작하자마자 스폰할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bAutoStart = true;

	//헌호수정 - 동시에 살아있을 수 있는 최대 좀비 수 (이 수 넘으면 스폰 스킵, 성능/안정성)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 MaxAliveZombies = 50;

	//헌호수정 - 타입별 스폰 가중치 (에디터에서 조정 가능. 합이 100 아니어도 비율로 계산)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning|TypeWeight")
	TMap<EVGMonsterType, float> TypeWeights;
	
	// 비어있으면 해당 몬스터 BP의 기본 랜덤 범위 사용, 값이 있으면 이 타입들 중에서만 랜덤
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<EVGMonsterType> AllowedMonsterTypes;

	// 외부(문 등)에서 스폰 발동
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ActivateSpawner();
	
	float LastSpawnCheckTime = -1.0f;

	void SpawnRandomMonster();
	FMonsterSpawnRow* GetRandomMonster() const;

protected:
	void BeginPlay() override;

private:
	void SpawnMonsters(int32 Count);
	void SpawnMonster(TSubclassOf<AActor> MonsterClass);
	bool bActivated = false;

	//헌호수정 - 주기적 스폰용 타이머 + 시작 시각
	FTimerHandle WaveTimerHandle;
	float SpawnStartTime = 0.f;

	//헌호수정 - 타이머가 호출: 경과 시간 계산해서 UpdateWave 실행
	void TickWave();

	//헌호수정 - 현재 살아있는 좀비 수 세기 (상한 체크용)
	int32 CountAliveZombies() const;

	//헌호수정 - 가중치로 좀비 타입 하나 뽑기
	EVGMonsterType PickWeightedType() const;
};
