// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "VGMonsterWalker.generated.h"

class UVGMonsterAttributeSet;

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Patrol,
	Stop,
	Chase,
	Attack,
	Dead
};

UCLASS()
class NAKWONCLONE_API AVGMonsterWalker : public AVGMonsterCharacterBase
{
	GENERATED_BODY()

public:
	AVGMonsterWalker();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TObjectPtr<UVGMonsterAttributeSet> MonsterAttributeSet;
	
#pragma region 워커 고유 스탯
	// 이동 속도 ( 추후 GAS로 옮길 예정 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Walker")
	float WalkSpeed = 100.f;
#pragma endregion

#pragma region 큐브 시각화
	UPROPERTY(VisibleAnywhere, Category = "Monster|Visual")
	UStaticMeshComponent* BodyMesh;

	UPROPERTY(EditAnywhere, Category = "Monster|Visual")
	UMaterialInterface* PatrolMaterial;		// 순찰 (파랑)

	UPROPERTY(EditAnywhere, Category = "Monster|Visual")
	UMaterialInterface* StoptMaterial;		// 정지 (하양)

	UPROPERTY(EditAnywhere, Category = "Monster|Visual")
	UMaterialInterface* ChaseMaterial;		// 추적 (초록)

	UPROPERTY(EditAnywhere, Category = "Monster|Visual")
	UMaterialInterface* AttackMaterial;		// 공격 (빨강)

	UPROPERTY(EditAnywhere, Category = "Monster|Visual")
	UMaterialInterface* DeadMaterial;		// 사망 (검정)
#pragma endregion
	
public:
	// BTTask에서 상태 변경 시 호출
	void SetMonsterState(EMonsterState NewState);
};