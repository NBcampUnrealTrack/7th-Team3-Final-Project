#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Player/PlayerData/NCWeaponData.h"

#include "NCInventoryType.generated.h"

class ANCItemActor;

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSubclassOf<ANCItemActor> ItemActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UStaticMesh* ItemMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UTexture2D* ItemIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGameplayTag ItemTypeTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize;
	
	FItemData()
		: ItemMesh(nullptr)
		, ItemIcon(nullptr)
		, ItemTypeTag(FGameplayTag::EmptyTag)
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
	FGameplayTag ItemTypeTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Slot")
	int32 Quantity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory Slot")
	FNCWeaponInstance WeaponInstance;
	
	FInventorySlot()
		: ItemID(NAME_None)
		, ItemTypeTag(FGameplayTag::EmptyTag)
		, Quantity(0)
	{}
	
	bool IsEmpty() const
	{
		return ItemID.IsNone() || !ItemTypeTag.IsValid() || Quantity <= 0;
	}
};

USTRUCT(BlueprintType)
struct FConsumableItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float HealAmount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float StaminaAmount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float InfectionReduceAmount = 0.f;
};

USTRUCT(BlueprintType)
struct FCreditItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credit")
	int32 MinValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Credit")
	int32 MaxValue = 0;
};