#include "NCTimerText.h"

#include "Framwork/GameState/NCGameState.h"

void UNCTimerText::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ANCGameState* GS = GetWorld()->GetGameState<ANCGameState>())
	{
		if (GS)
		{
			UE_LOG(LogTemp, Warning, TEXT("Bind Success"));
			GS->OnRemainingMatchTimeUpdate.AddDynamic(
				this,
				&UNCTimerText::OnTimerTextChanged
				);
		}
		
		OnTimerTextChanged(GS->RemainingMatchTime);
	}
}

void UNCTimerText::OnTimerTextChanged(int32 InToRemainingMatchTime)
{
	UE_LOG(LogTemp, Warning, TEXT("UI Received : %d"), InToRemainingMatchTime);
	
	int32 Minutes = InToRemainingMatchTime / 60;
	int32 Seconds = InToRemainingMatchTime % 60;

	TimerText->SetText(
		FText::FromString(
			FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)
		)
	);
}
