// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "AbilitySystemInterface.h"
#include "VGMonsterCharacterBase.generated.h"

UCLASS()
class NAKWONCLONE_API AVGMonsterCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AVGMonsterCharacterBase();
	
protected:
	virtual void BeginPlay() override;
	
	// AI 컨트롤러 참조 (읽기 전용, 블랙보드 직접 접근 금지)
	UPROPERTY()
	AVGMonsterAIControllerBase* AIController;
	
#pragma region ASC
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UVGMonsterAttributeSet> MonsterAttributeSet;
#pragma endregion
	
#pragma region 애니메이션
protected:
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimMove;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimStop;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimChase;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimAttack;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimHit;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimDead;
	
	UAnimMontage* GetRandomMontage(const TArray<TObjectPtr<UAnimMontage>>& Montages);
#pragma endregion
	
#pragma region 피격
public:
	UFUNCTION()
	void HandleHit();
	
private:
	bool bIsHit = false;
	FTimerHandle HitTimerHandle;
#pragma endregion
	
#pragma region 사망 처리
public:
	UFUNCTION()
	void HandleDead();
#pragma endregion
};