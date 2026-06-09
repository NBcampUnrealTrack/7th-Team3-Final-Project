#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "NCLootDropData.generated.h"

USTRUCT(BlueprintType)
struct FNCLootDropData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0"))
	float DropWeight; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxQuantity;

	FNCLootDropData()
		: ItemID(NAME_None), DropWeight(10.0f), MinQuantity(1), MaxQuantity(1)
	{}
};
