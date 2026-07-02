// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "VGMonsterRunner.generated.h"

class UVGMonsterAttributeSet;
class UGameplayEffect;

UCLASS()
class NAKWONCLONE_API AVGMonsterRunner : public AVGMonsterCharacterBase
{
	GENERATED_BODY()

#pragma region 코어/라이프사이클
public:
	AVGMonsterRunner();

protected:
	virtual void BeginPlay() override;
#pragma endregion
	
#pragma region 애니메이션
public:
	UAnimMontage* GetRandomAttackMontage() { return GetRandomMontage(AnimAttack); }

protected:
	// 워커 전용 Attack 애니메이션
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimAttack;
#pragma endregion
};