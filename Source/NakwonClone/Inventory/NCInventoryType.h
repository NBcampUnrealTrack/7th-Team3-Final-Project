#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "NCInventoryType.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGameplayTag ItemTypeTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize;
	
	FItemData()
		: ItemTypeTag(FGameplayTag::EmptyTag)
		, MaxStackSize(1)
	{}
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
	FGameplayTag ItemTypeTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
	int32 Quantity;
	
	FInventorySlot()
		: ItemTypeTag(FGameplayTag::EmptyTag)
		, Quantity(0)
	{}
	
	bool IsEmpty() const
	{
		return !ItemTypeTag.IsValid() || Quantity <= 0;
	}
};