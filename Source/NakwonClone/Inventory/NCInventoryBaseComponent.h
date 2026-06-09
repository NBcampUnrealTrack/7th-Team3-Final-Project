#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NCInventoryType.h"

#include "NCInventoryBaseComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCInventoryBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCInventoryBaseComponent();

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnInventoryUpdated OnInventoryUpdated;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_Items();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	FIntPoint GridSize;
	
	UPROPERTY(ReplicatedUsing = OnRep_Items, EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TArray<FInventorySlot> Items;
	
	UPROPERTY(ReplicatedUsing = OnRep_Items, EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TArray<FInventorySlot> EquipmentItem;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void InitializeInventory();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TObjectPtr<class UDataTable> ItemDataTable;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Helper")
    bool GetItemDataByTag(FName ItemID, FGameplayTag ItemTag, FItemData& OutItemData) const;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual bool AddItem(FName ItemID, FGameplayTag ItemTypeTag, int32 Quantity);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool RemoveItem(int32 SlotIndex, int32 Quantity);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool MoveItem(int32 FromIndex, int32 ToIndex);
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool FindEmptySlot(int32& OutSlotIndex) const;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool FindStackableSlot(FName ItemID, FGameplayTag ItemTypeTag, int32 MaxStackSize, int32& OutSlotIndex) const;
	
protected:
	bool SwapSlots(int32 IndexA, int32 IndexB);
	bool CombineSlots(int32 SourceIndex, int32 TargetIndex);
};