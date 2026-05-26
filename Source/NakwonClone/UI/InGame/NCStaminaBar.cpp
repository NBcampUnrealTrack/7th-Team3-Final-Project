#include "NCStaminaBar.h"

void UNCStaminaBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 캐릭터 델리게이트 연결
}

void UNCStaminaBar::OnStaminaBarChanged(float CurrentStamina, float MaxStamina)
{
	if (StaminaProgressBar && AssistanceStaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(CurrentStamina / MaxStamina);
		
		if (CurrentStamina >= MaxStamina)
		{
			StaminaCanvasPanel->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			StaminaCanvasPanel->SetVisibility(ESlateVisibility::Visible);
		}
		
		GetWorld()->GetTimerManager().SetTimer(
			OnStaminaBarTimerHandle,
			this,
			&UNCStaminaBar::OnAssistanceStaminaBarChanged,
			0.1f,
			false
		);
	}
}

void UNCStaminaBar::OnAssistanceStaminaBarChanged()
{
	// ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwningPlayer());
	
	// AssistanceStaminaProgressBar->SetPercent(Player->CurrentStamina / Player->MaxStamina);
}