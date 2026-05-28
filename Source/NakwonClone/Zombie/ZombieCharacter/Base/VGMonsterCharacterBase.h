// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "VGMonsterCharacterBase.generated.h"

UCLASS()
class NAKWONCLONE_API AVGMonsterCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AVGMonsterCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 스탯 (추후 확장)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stats")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|Stats")
	float CurrentHealth;

	// 피격 처리 (AIController 또는 외부에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	virtual void TakeDamage_Monster(float DamageAmount);

	// 사망 처리
	UFUNCTION(BlueprintCallable, Category = "Monster|Combat")
	virtual void Dead();

protected:
	// AI 컨트롤러 참조 (읽기 전용, 블랙보드 직접 접근 금지)
	UPROPERTY()
	AVGMonsterAIControllerBase* AIController;
};