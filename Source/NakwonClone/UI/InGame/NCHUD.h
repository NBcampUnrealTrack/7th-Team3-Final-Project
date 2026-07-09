#pragma once

#include "CoreMinimal.h"
#include "NCHPBar.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "NCHud.generated.h"

class UNHPBar;
class UNCStaminaBar;
class UAbilitySystemComponent;
class UVGPlayerAttributeSet;
class UImage; //헌호수정
class USoundBase; //헌호수정
class UAudioComponent; //헌호수정

UCLASS()
class NAKWONCLONE_API UNCHud : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override; //헌호수정 - 피 오버레이 페이드 처리

	void OnHPChanged(const FOnAttributeChangeData& Data);
	void OnStaminaChanged(const FOnAttributeChangeData& Data);

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UVGPlayerAttributeSet> AttributeSet;

	//헌호수정 - 피격 화면 효과 (콜오브듀티 스타일)
	// 화면 전체를 덮는 피 오버레이 이미지 (WBP_NCHUD에서 바인딩, optional로 없어도 크래시 X)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BloodOverlay;

	// 저체력 기준 (이 값 이하면 피 효과 계속 유지)
	UPROPERTY(EditAnywhere, Category = "BloodEffect")
	float LowHealthThreshold = 30.f;

	// 저체력 시 유지되는 최소 투명도
	UPROPERTY(EditAnywhere, Category = "BloodEffect")
	float LowHealthOpacity = 0.6f;

	// 피격 순간 확 올라가는 투명도
	UPROPERTY(EditAnywhere, Category = "BloodEffect")
	float HitFlashOpacity = 1.f;

	// 피 효과 사라지는 속도 (초당)
	UPROPERTY(EditAnywhere, Category = "BloodEffect")
	float BloodFadeSpeed = 2.f;

	// 현재 피 오버레이 투명도 (매 프레임 보간)
	float CurrentBloodOpacity = 0.f;

	//헌호수정 - 저체력 사운드 (심장박동 루프 + 숨소리)
	// 심장박동 (루프 재생, 체력 낮을수록 피치↑)
	UPROPERTY(EditAnywhere, Category = "LowHealthSound")
	TObjectPtr<USoundBase> HeartbeatSound;

	// 숨가쁜 소리 (일정 간격마다 1회씩)
	UPROPERTY(EditAnywhere, Category = "LowHealthSound")
	TObjectPtr<USoundBase> BreathSound;

	// 체력이 LowHealthThreshold일 때 심박 피치 배율
	UPROPERTY(EditAnywhere, Category = "LowHealthSound")
	float HeartbeatMinPitch = 1.f;

	// 체력이 0에 가까울 때 심박 피치 배율
	UPROPERTY(EditAnywhere, Category = "LowHealthSound")
	float HeartbeatMaxPitch = 1.8f;

	// 숨소리 재생 간격 최소/최대 (초)
	UPROPERTY(EditAnywhere, Category = "LowHealthSound")
	float BreathIntervalMin = 4.f;

	UPROPERTY(EditAnywhere, Category = "LowHealthSound")
	float BreathIntervalMax = 6.f;

private:
	//헌호수정 - 저체력 사운드 상태
	UPROPERTY()
	TObjectPtr<UAudioComponent> HeartbeatAudioComp;

	bool bLowHealthActive = false;  // 현재 저체력 사운드 재생 중인지
	FTimerHandle BreathTimerHandle;

	void StartLowHealthSound();     // 저체력 진입
	void StopLowHealthSound();      // 저체력 해제
	void UpdateHeartbeatPitch(float Health); // 체력에 따라 심박 피치 갱신
	void PlayBreathSound();         // 숨소리 1회 재생 + 다음 타이머 예약
};
