#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "NCStaminaBar.generated.h"

UCLASS()
class NAKWONCLONE_API UNCStaminaBar : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* AssistanceStaminaProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* StaminaCanvasPanel;
	
	UFUNCTION()
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void UpdateStaminaBar(float CurrentStamina, float MaxStamina);
	void UpdateAssistanceStaminaBar();
	
	FTimerHandle OnStaminaBarTimerHandle;
};
