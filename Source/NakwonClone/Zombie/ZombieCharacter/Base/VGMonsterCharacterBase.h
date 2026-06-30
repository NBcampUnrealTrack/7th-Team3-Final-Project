// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGHitTypes.h"
#include "VGMonsterCharacterBase.generated.h"

// 전방 선언
class AVGMonsterAIControllerBase;
class UAbilitySystemComponent;
class UVGMonsterAttributeSet;
class UAnimMontage;
class UCapsuleComponent;
class USoundBase;
class USoundAttenuation;
class UNiagaraSystem;
class UGameplayEffect;

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
	
	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> MoveSpeedEffectClass;

	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> ChaseSpeedEffectClass;

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UVGMonsterAttributeSet> MonsterAttributeSet;

#pragma endregion

#pragma region 애니메이션
public:
	UAnimMontage* GetRandomMontage(const TArray<TObjectPtr<UAnimMontage>>& Montages);
	UAnimMontage* GetRandomMoveMontage() { return GetRandomMontage(AnimMove); }
	UAnimMontage* GetRandomStopMontage() { return GetRandomMontage(AnimStop); }
	UAnimMontage* GetRandomChaseMontage() { return GetRandomMontage(AnimChase); }
	UAnimMontage* GetRandomAttackMontage() { return GetRandomMontage(AnimAttack); }
	UAnimMontage* GetRandomHitMontage() { return GetRandomMontage(AnimHit); }
	UAnimMontage* GetRandomDeadMontage() { return GetRandomMontage(AnimDead); }

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

public:
	int32 GetSelectedMoveLevel() const { return SelectedMoveLevel; }
	int32 GetSelectedChaseLevel() const { return SelectedChaseLevel; }

protected:
	UPROPERTY()
	int32 SelectedMoveLevel;

	UPROPERTY()
	int32 SelectedChaseLevel;
#pragma endregion
	
#pragma region 좀비 메시
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TArray<USkeletalMesh*> RandomMesh;
#pragma endregion
	
#pragma region 피격 처리
public:
	UFUNCTION()
	void HandleHit(const FVGHitData& HitData);

	// 부위별 피격 몽타주(에디터에서 머리 / 상체 / 하체별로 채움.비면 AnimHit로 폴백)
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TMap<EVGHitBodyPart, FVGHitMontageList> HitMontagesByPart;

	// 피격 후 몽타주 재생까지 딜레이 (0 = 즉시)
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	float HitReactDelay = 0.f;

	// 연속 피격 시 이전 몽타주 블렌드아웃 시간
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	float HitReactBlendOutTime = 0.1f;

protected:
	UAnimMontage* GetRandomHitMontageByPart(EVGHitBodyPart BodyPart);
	void PlayHitReactMontage(EVGHitBodyPart BodyPart);
	void OnHitReactDelayElapsed();

	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentHitMontage;

	EVGHitBodyPart PendingHitBodyPart = EVGHitBodyPart::None;

	FTimerHandle HitReactTimerHandle;
#pragma endregion

#pragma region 사망 처리
public:
	UFUNCTION()
	void HandleDead();

	void OnStartRagdoll();

	UFUNCTION(BlueprintImplementableEvent, Category = "Monster|Death")
	void OnStartDissolve();
	
private:
	bool bIsDead = false;

#pragma endregion

private:
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

#pragma region 충돌 감지
protected:
	UPROPERTY(VisibleAnywhere, Category = "Monster|Detection")
	TObjectPtr<UCapsuleComponent> DetectionCapsule;

	UFUNCTION()
	virtual void OnDetectionOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
#pragma endregion

#pragma region 사운드 
	//H
public:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySound(USoundBase* Sound);

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TMap<EVGHitBodyPart, TObjectPtr<USoundBase>> HitSoundsByPart;

	// 부위별 피격 VFX (타격 위치에 스폰)
	UPROPERTY(EditAnywhere, Category = "Monster|VFX")
	TMap<EVGHitBodyPart, TObjectPtr<UNiagaraSystem>> HitVFXByPart;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHitVFX(UNiagaraSystem* VFX, FVector Location);

protected:
	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TObjectPtr<USoundBase> DeathSound;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TObjectPtr<USoundBase> DetectSound;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TObjectPtr<USoundBase> HowlSound;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TObjectPtr<USoundAttenuation> SoundAttenuation;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	float HowlIntervalMin = 6.f;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	float HowlIntervalMax = 14.f;

	USoundBase* GetHitSoundByPart(EVGHitBodyPart BodyPart) const;
	UNiagaraSystem* GetHitVFXByPart(EVGHitBodyPart BodyPart) const;

	FTimerHandle HowlTimerHandle;

	void StartHowlTimer();
	void HandleHowl();
	// bool bIsDead = false;
#pragma endregion

#pragma region 암살 처리
public:
	// 플레이어가 암살 시작 시 호출 (서버에서). 재생할 victim 몽타주는 DT에서 받음
	void BeginAssassinationVictim(AActor* Killer, UAnimMontage* VictimMontage);

	// 좀비 등 → 플레이어 정면이 되도록 맞출 거리
	UPROPERTY(EditAnywhere, Category = "Monster|Assassination")
	float AssassinationAlignDistance = 90.f;

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAssassinationMontage(UAnimMontage* Montage);

	bool bIsBeingAssassinated = false;
#pragma endregion
};