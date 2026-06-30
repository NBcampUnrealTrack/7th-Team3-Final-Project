// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "VGMonsterRunner.generated.h"

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
	
#pragma region GAS
public:
	// 공격 GE 슬롯 (워커 전용)
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	TSubclassOf<class UGameplayEffect> AttackEffectClass;
#pragma endregion
	
#pragma region 애니메이션
public:
	UAnimMontage* GetRandomAttackMontage() { return GetRandomMontage(AnimAttack); }

protected:
	// 워커 전용 Attack 애니메이션
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimAttack;
#pragma endregion

#pragma region 공격 트레이스
public:
	const TArray<FName>& GetAttackSocketNames() const { return AttackSocketNames; }
	float GetAttackTraceDistance() const { return AttackTraceDistance; }
	
private:
	// 소켓 이름 (스켈레톤 에디터에서 추가한 이름과 동일하게)
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	TArray<FName> AttackSocketNames = {
		TEXT("AttackSocket_L_Fist"),
		TEXT("AttackSocket_L_Wrist"),
		TEXT("AttackSocket_L_Elbow"),
		TEXT("AttackSocket_R_Fist"),
		TEXT("AttackSocket_R_Wrist"),
		TEXT("AttackSocket_R_Elbow"),
	};
	
	// 트레이스 크기
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	float AttackTraceDistance = 10.f;
	
#pragma endregion
};
