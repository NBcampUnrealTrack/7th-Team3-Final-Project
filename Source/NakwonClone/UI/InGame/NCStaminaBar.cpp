#include "NCStaminaBar.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"

void UNCStaminaBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(GetOwningPlayerPawn());
	
	if (PlayerCharacter)
	{
		PlayerCharacter->OnStaminaBarChanged.AddDynamic(this, &UNCStaminaBar::UpdateStaminaBar);
	}
}

void UNCStaminaBar::UpdateStaminaBar(float CurrentStamina, float MaxStamina)
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
			&UNCStaminaBar::UpdateAssistanceStaminaBar,
			0.1f,
			false
		);
	}
}

void UNCStaminaBar::UpdateAssistanceStaminaBar()
{
	// ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwningPlayer());
	
	// AssistanceStaminaProgressBar->SetPercent(Player->CurrentStamina / Player->MaxStamina);
}