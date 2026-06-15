#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "NCTimerText.generated.h"

UCLASS()
class NAKWONCLONE_API UNCTimerText : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimerText;
	
	UFUNCTION()
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnTimerTextChanged(int32 InToRemainingMatchTime);
};
