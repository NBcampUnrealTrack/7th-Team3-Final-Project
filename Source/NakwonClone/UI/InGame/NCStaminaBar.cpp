#include "NCStaminaBar.h"

#include "NakwonClone/Framwork/PlayerState/NCPlayerState.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"

void UNCStaminaBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateAssistanceStaminaBar(InDeltaTime);
}

void UNCStaminaBar::UpdateStaminaBar(float InToCurrentStamina, float InToMaxStamina)
{
	MaxStamina = InToMaxStamina;
	
	if (StaminaProgressBar && AssistanceStaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(InToCurrentStamina / MaxStamina);
		
		TargetStamina = InToCurrentStamina;
		
		if (TargetStamina >= InToMaxStamina)
		{
			StaminaProgressBar->SetVisibility(ESlateVisibility::Hidden);
			AssistanceStaminaProgressBar->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			StaminaProgressBar->SetVisibility(ESlateVisibility::Visible);
			AssistanceStaminaProgressBar->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UNCStaminaBar::UpdateAssistanceStaminaBar(float DeltaTime)
{
	if (MaxStamina <= 0.f) return;
	
	CurrentAssistStamina = FMath::FInterpTo(CurrentAssistStamina, TargetStamina, DeltaTime, 6.f);
	
	AssistanceStaminaProgressBar->SetPercent(CurrentAssistStamina / MaxStamina);
}