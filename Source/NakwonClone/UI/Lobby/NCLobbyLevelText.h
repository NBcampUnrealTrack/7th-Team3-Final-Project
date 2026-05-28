#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "NCLobbyLevelText.generated.h"

UCLASS()
class NAKWONCLONE_API UNCLobbyLevelText : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExpText;
	
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ExpProgressBar;
	
	UFUNCTION()
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void UpdateLevelText(int32 CharacterLevel);
};
