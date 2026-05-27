#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "NCHPBar.generated.h"

UCLASS()
class NAKWONCLONE_API UNCHPBar : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPProgressBar;

	UFUNCTION()
	void NativeConstruct();
	
	UFUNCTION()
	void UpdateHP(float CurrentHP, float MaxHP);
};
