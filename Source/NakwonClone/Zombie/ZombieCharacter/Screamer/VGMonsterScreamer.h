// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "VGMonsterScreamer.generated.h"

class UAnimMontage;

UCLASS()
class NAKWONCLONE_API AVGMonsterScreamer : public AVGMonsterCharacterBase
{
	GENERATED_BODY()

public:
	AVGMonsterScreamer();

protected:
	virtual void BeginPlay() override;

#pragma region 스크림(절규) 능력
public:
	// BT(BTTask_Scream)나 외부에서 호출하는 진입점. 서버에서만 실제 실행됨.
	UFUNCTION(BlueprintCallable, Category = "Screamer|Scream")
	void RequestScream(AActor* TargetActor);

	// 지금 비명을 지를 수 있는지 (BT 데코레이터/태스크 조건용)
	UFUNCTION(BlueprintPure, Category = "Screamer|Scream")
	bool CanScream() const;

protected:
	// 실제 비명 처리 (서버 권위)
	void PerformScream(AActor* TargetActor);

	// 비명 몽타주 동기 재생 (모든 클라)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayScreamMontage(UAnimMontage* Montage);

	void OnScreamCooldownElapsed();

	// 자동 비명 루프 (BT가 아직 안 붙었을 때도 동작하게 하는 안전장치)
	void AutoScreamCheck();

protected:
	// 비명 애니메이션 (비면 미재생)
	UPROPERTY(EditAnywhere, Category = "Screamer|Animation")
	TArray<TObjectPtr<UAnimMontage>> AnimScream;

	// 절규 사운드 (HowlSound와 별개의 더 크고 강한 비명. 비면 HowlSound로 폴백)
	UPROPERTY(EditAnywhere, Category = "Screamer|Sound")
	TObjectPtr<USoundBase> ScreamSound;

	// 비명이 닿는 반경 — 이 안의 좀비가 각성/유인됨
	UPROPERTY(EditAnywhere, Category = "Screamer|Scream")
	float ScreamRadius = 3000.f;

	// 청각 노이즈 이벤트 크기 (AIPerception Hearing으로 전달)
	UPROPERTY(EditAnywhere, Category = "Screamer|Scream")
	float ScreamLoudness = 3.f;

	// 재사용 대기시간(초)
	UPROPERTY(EditAnywhere, Category = "Screamer|Scream")
	float ScreamCooldown = 8.f;

	// 자동 비명 루프 사용 여부. ysh의 BTTask_Scream가 붙으면 false로 꺼도 됨.
	UPROPERTY(EditAnywhere, Category = "Screamer|Scream")
	bool bAutoScream = true;

	// 자동 비명 체크 주기(초)
	UPROPERTY(EditAnywhere, Category = "Screamer|Scream")
	float AutoScreamCheckInterval = 1.0f;

private:
	bool bScreamOnCooldown = false;
	FTimerHandle ScreamCooldownTimer;
	FTimerHandle AutoScreamTimer;
#pragma endregion
};