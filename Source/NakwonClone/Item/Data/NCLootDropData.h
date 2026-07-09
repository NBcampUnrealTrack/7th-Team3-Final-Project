#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "NCLootDropData.generated.h"

USTRUCT(BlueprintType)
struct FNCLootDropData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 소모품 드랍용 (DT_ItemTypeData의 ItemID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName ItemID;
	
	// 총기 드랍용 (DT_NCCGunData의 GunID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName GunID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0"))
	float DropWeight; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinQuantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxQuantity;

	FNCLootDropData()
		: ItemID(NAME_None), GunID(NAME_None), DropWeight(10.0f), MinQuantity(1), MaxQuantity(1)
	{}
};
