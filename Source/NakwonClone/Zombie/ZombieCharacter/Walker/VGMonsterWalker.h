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
	
#pragma region 애니메이션
protected:
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedWakeUpMontage;
	
public:
	UAnimMontage* GetSleepMontage() const { return AnimSleep; }
	UAnimMontage* GetSelectedWakeUpMontage() const { return SelectedWakeUpMontage; }

protected:
	// 워커 전용 Idle 애니메이션
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimSleep;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimWakeUp;
#pragma endregion
	
	
#pragma region 좀비 메시
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TArray<USkeletalMesh*> RandomMesh;
#pragma endregion

#pragma region 공격 트레이스
public:
	void PerformAttackTrace();

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
	
	// 트레이스 거리
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	float AttackTraceDistance = 100.f;
#pragma endregion
	
#pragma region GAS GE 슬롯
public:
	// 에디터에서 GE_Attack 에셋 할당
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	TSubclassOf<class UGameplayEffect> AttackEffectClass;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> WalkSpeedEffectClass;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> ChaseSpeedEffectClass;
#pragma endregion
	
#pragma region WakeUp
public:
	virtual void OnDetectionOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult) override;
	
	// 랜덤 딜레이
	void WakeUpWithDelay();
	
private:
	void WakeUp();
	FTimerHandle WakeUpTimerHandle;
#pragma endregion
};