// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGHitTypes.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterTypeData.h" 
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
class ANCItemActor;

DECLARE_DELEGATE(FOnMonsterAttackFinished);

struct FOnAttributeChangeData;

UCLASS()
class NAKWONCLONE_API AVGMonsterCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

#pragma region 코어/라이프사이클
public:
	// 생성자
	AVGMonsterCharacterBase();
protected:
	// 오버라이드
	virtual void BeginPlay() override;

	// AI 컨트롤러 참조 (읽기 전용, 블랙보드 직접 접근 금지)
	UPROPERTY()
	AVGMonsterAIControllerBase* AIController;
#pragma endregion

#pragma region GAS
public:
	// 인터페이스 구현
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UVGMonsterAttributeSet* GetMonsterAttributeSet() const { return MonsterAttributeSet; }

protected:
	// ASC 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "GAS|ASC")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	// 몬스터 어트로뷰트셋
	UPROPERTY(VisibleAnywhere, Category = "GAS|AttributeSet")
	TObjectPtr<UVGMonsterAttributeSet> MonsterAttributeSet;

public:
	// 공격 (물기) GE 슬롯
	UPROPERTY(EditAnywhere, Category = "Monster|Bite")
	TSubclassOf<UGameplayEffect> BiteEffectClass;

	// 속도 GE 슬롯
	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> MoveSpeedEffectClass;

	UPROPERTY(EditAnywhere, Category = "Monster|Speed")
	TSubclassOf<UGameplayEffect> ChaseSpeedEffectClass;

private:
	// 어트로뷰트 변경 콜백
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
#pragma endregion

#pragma region 애니메이션
public:
	// 랜덤 몽타주 가져오기
	UAnimMontage* GetRandomMontage(const TArray<TObjectPtr<UAnimMontage>>& Montages);

	// 슬롯 배열에서 랜덤 애니메이션 몽타주 추출
	UAnimMontage* GetRandomMoveMontage() { return GetRandomMontage(AnimMove); }
	UAnimMontage* GetRandomStopMontage() { return GetRandomMontage(AnimStop); }
	UAnimMontage* GetRandomChaseMontage() { return GetRandomMontage(AnimChase); }
	UAnimMontage* GetRandomHitMontage() { return GetRandomMontage(AnimHit); }

	// BeginPlay에서 한 번 골라 캐시한 몽타주
	UAnimMontage* GetSelectedMoveMontage() { return SelectedMoveMontage; }
	UAnimMontage* GetSelectedChaseMontage() { return SelectedChaseMontage; }
	UAnimMontage* GetSelectedStopMontage() { return SelectedStopMontage; }

	UFUNCTION(BlueprintCallable, Category = "Monster")
	UBlendSpace* GetLocomotionBS() const { return CachedLocomotionBS; }

	UFUNCTION(BlueprintPure, Category = "Monster")
	float GetAnimPlayRateScale() const { return AnimPlayRateScale; }

	UFUNCTION(BlueprintPure, Category = "Monster")
	float GetAnimStartPosition() const { return AnimStartPosition; }

	UPROPERTY(EditAnywhere, Category = "Animation")
	float LocomotionPlayRate = 1.f;

	// 원거리 캐시
	UPROPERTY() bool bIsRanged = false;
	UPROPERTY() TSubclassOf<AActor> ProjectileClass = nullptr;
	FName CachedProjectileSocket = NAME_None;
	float CachedAttackRange = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Monster|Ranged")
	float GetAttackRange() const { return CachedAttackRange; }

	// 침 발사 (공격 몽타주의 AnimNotify에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Monster|Ranged")
	void SpawnProjectile();

	UPROPERTY() TObjectPtr<UNiagaraSystem> ThrowVFX = nullptr;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnThrowVFX(const FVector& Location);

protected:
	// ── 몽타주 슬롯 (에디터에서 채움) ────────────────────────
	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimMove;

	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimStop;

	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimChase;

	UPROPERTY(EditAnywhere, Category = "Monster|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimHit;

	// ── 선택 캐시 (런타임) ────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedMoveMontage;

	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedChaseMontage;

	UPROPERTY()
	TObjectPtr<UAnimMontage> SelectedStopMontage;

	// ── 속도 레벨 ────────────────────────────────────────
public:
	int32 GetSelectedMoveLevel() const { return SelectedMoveLevel; }
	int32 GetSelectedChaseLevel() const { return SelectedChaseLevel; }

protected:
	UPROPERTY()
	int32 SelectedMoveLevel;

	UPROPERTY()
	int32 SelectedChaseLevel;

private:
	UPROPERTY()
	TObjectPtr<UBlendSpace> CachedLocomotionBS = nullptr;

	UPROPERTY()
	float AnimPlayRateScale = 1.f;

	UPROPERTY()
	float AnimStartPosition = 0.f;

#pragma endregion

#pragma region 좀비 메시 (랜덤)
protected:
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
	
	// 피격 시 넉백 재생 확률
	UPROPERTY()
	float HitReactChance;

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

	void SetLastDamageCauser(AActor* InCauser);

	void OnStartRagdoll();

	UFUNCTION(BlueprintImplementableEvent, Category = "Monster|Death")
	void OnStartDissolve();

	bool IsDead() const { return bIsDead; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Death")
	float DeathLifeSpan = 3.0f;
	
	// 사망 시 드랍할 소모품 테이블 (FNCLootDropData 행, 총기 제외)
	UPROPERTY(EditAnywhere, Category = "Monster|Loot")
	TObjectPtr<UDataTable> LootDropTable = nullptr;

private:
	bool bIsDead = false;

	UPROPERTY()
	TObjectPtr<AActor> LastDamageCauser = nullptr;
	
	void DropLoot();
#pragma endregion

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
public:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySound(USoundBase* Sound, USoundAttenuation* AttenuationOverride);

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	TMap<EVGHitBodyPart, TObjectPtr<USoundBase>> HitSoundsByPart;

	// 부위별 피격 VFX (타격 위치에 스폰)
	UPROPERTY(EditAnywhere, Category = "Monster|VFX")
	TMap<EVGHitBodyPart, TObjectPtr<UNiagaraSystem>> HitVFXByPart;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHitVFX(UNiagaraSystem* VFX, FVector Location);

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	float HitSoundCooldown = 0.1f;   // 이 시간 안엔 피격음 재생 안 함

	float LastHitSoundTime = -100.f; // 마지막 피격음 재생 시각

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
	TObjectPtr<USoundAttenuation> CombatAttenuation;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	float HowlIntervalMin = 6.f;

	UPROPERTY(EditAnywhere, Category = "Monster|Sound")
	float HowlIntervalMax = 14.f;

	UPROPERTY()
	TObjectPtr<USoundBase> IdleSound = nullptr;

	UPROPERTY()
	float IdleSoundCooldown = 5.f;

	FTimerHandle IdleTimerHandle;
	void StartIdleTimer();
	void HandleIdle();

	USoundBase* GetHitSoundByPart(EVGHitBodyPart BodyPart) const;
	UNiagaraSystem* GetHitVFXByPart(EVGHitBodyPart BodyPart) const;

	FTimerHandle HowlTimerHandle;

	void StartHowlTimer();
	void HandleHowl();
#pragma endregion

#pragma region 암살 처리
public:
	// 플레이어가 암살 시작 시 호출 (서버에서). 재생할 victim 몽타주는 DT에서 받음
	void BeginAssassinationVictim(AActor* Killer, UAnimMontage* VictimMontage);

	// 좀비 등 → 플레이어 정면이 되도록 맞출 거리
	UPROPERTY(EditAnywhere, Category = "Monster|Assassination")
	float AssassinationAlignDistance = 90.f;
	bool IsBeingAssassinated() const { return bIsBeingAssassinated; }

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAssassinationMontage(UAnimMontage* Montage);

	bool bIsBeingAssassinated = false;
#pragma endregion

#pragma region 타입/공격 (AI)
public:
	// 스폰 시 이 타입으로 세팅
	UPROPERTY(EditAnywhere, Category = "Monster|Type")
	EVGMonsterType MonsterType = EVGMonsterType::Walker;

	UPROPERTY(EditAnywhere, Category = "Monster|Type")
	TObjectPtr<UDataTable> MonsterTypeTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Monster|Type")
	bool bRandomType = false;

	// BT가 부르는 진입점 — "공격 시작" (재생은 캐릭터가)
	void StartAttack();

	// 공격 몽타주 끝나면 BT에 알림
	FOnMonsterAttackFinished OnAttackFinished;

	// ── 공격 트레이스/GE (AnimNotify_AttackTrace가 사용) ──────────
	TSubclassOf<UGameplayEffect> GetAttackEffectClass() const { return CachedAttackEffectClass; }
	const TArray<FName>& GetAttackSocketNames() const { return AttackSocketNames; }
	float GetAttackTraceDistance() const { return AttackTraceDistance; }

	float GetPatrolSpeed() const { return CachedPatrolSpeed; }

protected:
	// 타입 데이터 적용 (메시/ABP/스탯/공격몽타주 캐시)
	void ApplyMonsterType();

	// 현재 타입 공격 몽타주 중 하나
	UAnimMontage* GetAttackMontageForAI();

	// 소켓 이름 (스켈레톤 에디터에서 추가한 이름과 동일하게)
	UPROPERTY(EditAnywhere, Category = "Monster|Attack")
	TArray<FName> AttackSocketNames = {
		TEXT("AttackSocket_L_Fist"),
		TEXT("AttackSocket_L_Wrist"),
		TEXT("AttackSocket_L_Elbow"),
		TEXT("AttackSocket_R_Fist"),
		TEXT("AttackSocket_R_Wrist"),
		TEXT("AttackSocket_R_Elbow"),
	};

	// 트레이스 크기
	UPROPERTY(EditAnywhere, Category = "Monster|Attack")
	float AttackTraceDistance = 10.f;

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(UAnimMontage* Montage);

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 실제 공격 몽타주 재생 (내부용)
	void PlayAttackNow();

	// 타입 데이터 런타임 캐시
	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> CachedAttackMontages;

	UPROPERTY()
	TObjectPtr<UAnimMontage> CachedSpecialMontage = nullptr;

	// 타입별 공격 GE 캐시
	UPROPERTY()
	TSubclassOf<UGameplayEffect> CachedAttackEffectClass;
	
	UPROPERTY()
	float CachedPatrolSpeed = 100.f;

	UPROPERTY()
	TObjectPtr<UAnimMontage> CurrentPlayingMontage = nullptr;

	// Witch: 첫 공격 때 큰소리 1회
	bool bScreamPhase = false;
	bool bHasScreamed = false;
	
public: // 우정 추가
	UPROPERTY(BlueprintReadWrite)
	int32 CashedKillScore = 20;

public:
	// 큰소리(스페셜) 몽타주 재생 시도 — 접촉/피격 공용 진입점
	void TryPlaySpecialMontage();

	// 큰소리 재생 최소 간격(초) — 연속 피격 스팸 방지
	UPROPERTY(EditAnywhere, Category = "Monster|Special")
	float SpecialMontageCooldown = 3.f;

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpecialMontage(UAnimMontage* Montage);

	float LastSpecialMontageTime = -100.f;
	
public:
	// 타겟 주변 공격 슬롯 예약. 성공 시 OutSlotLocation에 이동할 좌표 기록
	bool ReserveAttackSlot(AActor* Target, FVector& OutSlotLocation);
	bool ReserveWaitSlot(AActor* Target, FVector& OutSlotLocation); 

	// 슬롯 반납 (사망/전투 이탈 시 호출)
	void ReleaseAttackSlot();
	
	bool GetReservedSlotLocation(FVector& OutLocation) const;

private:
	UPROPERTY()
	TWeakObjectPtr<class UVGAttackSlotComponent> ReservedSlotComp;
	
	UPROPERTY()
	int32 ReservedSlotIndex = -1;
	
	UPROPERTY()
	bool bReservedIsWaitSlot = false;

#pragma endregion
	
public:
	// bRandomType이 true일 때, 비어있지 않으면 이 목록 안에서만 랜덤 선택
	UPROPERTY(EditAnywhere, Category = "Monster|Type")
	TArray<EVGMonsterType> AllowedRandomTypes;
	
};