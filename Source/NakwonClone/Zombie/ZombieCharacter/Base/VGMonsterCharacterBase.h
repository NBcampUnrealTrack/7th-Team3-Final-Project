// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "VGMonsterCharacterBase.generated.h"

// 전방 선언
class AVGMonsterAIControllerBase;
class UAbilitySystemComponent;
class UVGMonsterAttributeSet;
class UAnimMontage;
class UCapsuleComponent;

struct FOnAttributeChangeData;

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
public:
	UAnimMontage* GetRandomMontage(const TArray<TObjectPtr<UAnimMontage>>& Montages);
	UAnimMontage* GetRandomMoveMontage()   { return GetRandomMontage(AnimMove); }
	UAnimMontage* GetRandomStopMontage()   { return GetRandomMontage(AnimStop); }
	UAnimMontage* GetRandomChaseMontage()  { return GetRandomMontage(AnimChase); }
	UAnimMontage* GetRandomAttackMontage() { return GetRandomMontage(AnimAttack); }
	UAnimMontage* GetRandomHitMontage()    { return GetRandomMontage(AnimHit); }
	UAnimMontage* GetRandomDeadMontage()   { return GetRandomMontage(AnimDead); }
	
	UAnimMontage* GetSelectedMoveMontage() { return SelectedMoveMontage; }
	UAnimMontage* GetSelectedChaseMontage() { return SelectedChaseMontage; }
	UAnimMontage* GetSelectedStopMontage() { return SelectedStopMontage; }
	UAnimMontage* GetSelectedDeadMontage() { return SelectedDeadMontage; }
	
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
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedMoveMontage;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedChaseMontage;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedStopMontage;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedDeadMontage;
#pragma endregion
	
#pragma region 피격 처리
public:
	UFUNCTION()
	void HandleHit();
#pragma endregion
	
#pragma region 사망 처리
public:
	UFUNCTION()
	void HandleDead();
	
	void OnStartRagdoll();
#pragma endregion
	
#pragma region GAS GE 슬롯
public:
	// 에디터에서 GE_Attack 에셋 할당
	UPROPERTY(EditAnywhere, Category = "Walker|Attack")
	TSubclassOf<class UGameplayEffect> AttackEffectClass;
	
	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> SpeedEffectClass;
#pragma endregion 	
	
private:
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
	
#pragma region 충돌 감지
protected:
	UPROPERTY(VisibleAnywhere, Category = "Monster|Detection")
	TObjectPtr<UCapsuleComponent> DetectionCapsule;
	
	UFUNCTION()
	void OnDetectionOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep, 
		const FHitResult& SweepResult);
	
public:
	// 랜덤 딜레이
	void WakeUpWithDelay();
	
private:
	void WakeUp();
	FTimerHandle WakeUpTimerHandle;
	
#pragma endregion
};