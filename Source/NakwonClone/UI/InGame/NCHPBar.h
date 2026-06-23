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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> LeftSpacer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USpacer> RightSpacer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;

public:
	UFUNCTION()
	void UpdateHP(float CurrentHP, float MaxHP);
};
