#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NCInventoryType.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Equipment UMETA(Displayname = "장비"),
	Consumable UMETA(Displayname = "소모품"),
	Material UMETA(Displayname = "재료")
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemType ItemType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize;
	
	FItemData()
		: ItemID(NAME_None)
		, ItemType(EItemType::Consumable)
		, MaxStackSize(1)
	{}
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
	FName ItemID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
	int32 Quantity;
	
	FInventorySlot()
		: ItemID(NAME_None)
		, Quantity(0)
	{}
	
	bool IsEmpty() const
	{
		return ItemID.IsNone() || Quantity <= 0;
	}
};