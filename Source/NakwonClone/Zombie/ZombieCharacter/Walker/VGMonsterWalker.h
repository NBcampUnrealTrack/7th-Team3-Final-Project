// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "VGMonsterWalker.generated.h"

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
	
#pragma region 워커 고유 스탯
	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Walker")
	float WalkSpeed = 100.f;
	// 감염 ( 감염 수치 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Walker")
	float InfectionFigures = 10.0f;
	// 공격력 ( 공격 시 깎는 체력 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Walker")
	float AttackDamage = 15.0f;
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
	// Walker 고유 피격 반응 (필요 시 오버라이드)
	virtual void TakeDamage_Monster(float DamageAmount) override;
	
	// BTTask에서 상태 변경 시 호출
	void SetMonsterState(EMonsterState NewState);
};