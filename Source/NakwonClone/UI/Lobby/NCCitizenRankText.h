#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "NCCitizenRankText.generated.h"

UCLASS()
class NAKWONCLONE_API UNCCitizenRankText : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CitizenRankText;
};
