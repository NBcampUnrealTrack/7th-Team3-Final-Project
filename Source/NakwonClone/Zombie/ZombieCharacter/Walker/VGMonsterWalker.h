// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "VGMonsterWalker.generated.h"

class UVGMonsterAttributeSet;
class UAnimMontage;

UCLASS()
class NAKWONCLONE_API AVGMonsterWalker : public AVGMonsterCharacterBase
{
	GENERATED_BODY()

public:
	AVGMonsterWalker();

protected:
	virtual void BeginPlay() override;
	
#pragma region 워커 고유 스탯
	// 이동 속도 ( 추후 GAS로 옮길 예정 )
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Walker")
	float WalkSpeed = 80.f;
#pragma endregion

#pragma region 공격 트레이스
public:
	void PerformAttackTrace();
	
private:
	// 에디터에서 GE_Attack 에셋 할당
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	TSubclassOf<class UGameplayEffect> AttackEffectClass;
	
	// 소켓 이름 (스켈레톤 에디터에서 추가한 이름과 동일하게)
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	TArray<FName> AttackSocketNames = {
		TEXT("AttackSocket_Fist"),
		TEXT("AttackSocket_Wrist"),
		TEXT("AttackSocket_Elbow")
	};
	
	// 트레이스 거리
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	float AttackTraceDistance = 100.f;
#pragma endregion
};