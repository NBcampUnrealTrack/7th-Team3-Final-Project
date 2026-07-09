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
	// 루팅박스 슬롯 표시용 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UTexture2D* LootBoxIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGameplayTag ItemTypeTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	int32 MaxStackSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FGameplayTagContainer EquipTags; 
	
	// 습득 1회성 이펙트/사운드 + 상시 아우라 (아이템별로 다르므로 데이터테이블에서 관리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Pickup")
	class UNiagaraSystem* PickupEffect;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Pickup")
	class USoundBase* PickupSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Pickup")
	class UNiagaraSystem* IdleAuraEffect;
	
	FItemData()
		: ItemMesh(nullptr)
		, ItemIcon(nullptr)
		, LootBoxIcon(nullptr)
		, ItemTypeTag(FGameplayTag::EmptyTag)
		, MaxStackSize(1)
		, PickupEffect(nullptr)
		, PickupSound(nullptr)
		, IdleAuraEffect(nullptr)
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

USTRUCT(BlueprintType)
struct FEquipmentPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FInventorySlot RightHand; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FInventorySlot LeftHand;  
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FInventorySlot TwoHand; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FInventorySlot ConsumableHeal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FInventorySlot ConsumableFood;

	bool IsTwoHandActive() const { return !TwoHand.IsEmpty(); }
	bool IsEmpty() const { return RightHand.IsEmpty() && LeftHand.IsEmpty() && TwoHand.IsEmpty(); }
};