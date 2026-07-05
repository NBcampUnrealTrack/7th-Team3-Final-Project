#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "NCHPBar.generated.h"

UCLASS()
class NAKWONCLONE_API UNCHPBar : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;

	//헌호수정 - 백팩용 심플 체력바(숫자/스페이서 없음)도 쓸 수 있게 옵셔널 처리
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USpacer> LeftSpacer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USpacer> RightSpacer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HPText;

public:
	UFUNCTION()
	void UpdateHP(float CurrentHP, float MaxHP);
};
