// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_AttackTrace.generated.h"


UCLASS()
class NAKWONCLONE_API UAnimNotify_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	// 적 히트 시 카메라 쉐이크 (무기별로 다르게 할당)
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> HitShakeClass;
	// 구간 시작: HitActors 초기화
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	// 구간 중 매 프레임: SphereTrace 히트 체크
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	// 구간 종료: HitActors 비우기
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	// 한 번의 공격에서 이미 맞은 액터 추적 (중복 히트 방지)
	TSet<AActor*> HitActors;

	void DoHitCheck(USkeletalMeshComponent* MeshComp);
};