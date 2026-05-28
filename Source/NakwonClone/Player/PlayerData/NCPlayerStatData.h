#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"  
#include "NCPlayerStatData.generated.h"

USTRUCT(BlueprintType)
struct FNCPlayerStatData : public FTableRowBase
{
    GENERATED_BODY()

    //체력 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxHP = 100.f;

    //스태미나 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxStamina = 100.f;

    //생존 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    bool bIsAlive = true;
};