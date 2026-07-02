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
#pragma region 코어/라이프사이클
public:
	AVGMonsterWalker();

protected:
	virtual void BeginPlay() override;
#pragma endregion
	
#pragma region 애니메이션
protected:
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedWakeUpMontage;
	
public:
	UAnimMontage* GetSleepMontage() const { return AnimSleep; }
	UAnimMontage* GetSelectedWakeUpMontage() const { return SelectedWakeUpMontage; }
	UAnimMontage* GetRandomAttackMontage() { return GetRandomMontage(AnimAttack); }
	
public:
	// 랜덤 공격 몽타주 재생. 재생한 몽타주 반환(실패 시 nullptr)
	UAnimMontage* Attack();

protected:
	// 워커 전용 Idle 애니메이션
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TObjectPtr<UAnimMontage> AnimSleep;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimWakeUp;
	
	// 워커 전용 Attack 애니메이션
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimAttack;
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