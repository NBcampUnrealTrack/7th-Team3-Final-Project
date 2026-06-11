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

	float TargetStamina;
	float CurrentAssistStamina;
	float MaxStamina;
	
public:
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	UFUNCTION()
	void UpdateStaminaBar(float InToCurrentStamina, float InToMaxStamina);
	void UpdateAssistanceStaminaBar(float DeltaTime);
};
