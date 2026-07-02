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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNCHPBar> HPBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNCStaminaBar> StaminaBarWidget;

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
};
