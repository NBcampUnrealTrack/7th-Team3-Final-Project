#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NCKeyGuideType.h"
#include "NCKeyGuideLibrary.generated.h"

class UDataTable;
class UTexture2D;

UCLASS()
class NAKWONCLONE_API UNCKeyGuideLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "KeyGuide")
    static UTexture2D* GetIconForKey(UDataTable* KeyIconTable, FKey Key);

    UFUNCTION(BlueprintCallable, Category = "KeyGuide")
    static bool GetTutorialStep(UDataTable* TutorialStepTable, FName StepID, FNCTutorialStepData& OutStepData);
};
