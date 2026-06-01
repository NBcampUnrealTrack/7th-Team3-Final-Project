// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "VGMonsterWalker.generated.h"

class UVGMonsterAttributeSet;
class UAnimMontage;

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	Move,
	Stop,
	Chase,
	Attack,
	Hit,
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

#pragma region 애니메이션
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimMove;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimStop;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimChase;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimHit;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimDead;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimAttack;
#pragma endregion
	
public:
	// BTTask에서 상태 변경 시 호출
	void SetMonsterState(EMonsterState NewState);
};