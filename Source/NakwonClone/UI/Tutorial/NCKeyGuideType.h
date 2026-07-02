#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"
#include "NCKeyGuideType.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FNCKeyIconRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyGuide")
    TObjectPtr<UTexture2D> Icon;
};

USTRUCT(BlueprintType)
struct FNCTutorialStepData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyGuide")
    TArray<FKey> RequiredKeys;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyGuide")
    FText GuideText;
};
