#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"

#include "NCPlayerInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsedSignature, FGameplayTag, UsedItemTag);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCPlayerInventoryComponent : public UNCInventoryBaseComponent
{
	GENERATED_BODY()

public:
	UNCPlayerInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnQuickSlotUpdated OnQuickSlotUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemUsedSignature OnItemUsed;
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    
	UFUNCTION()
	virtual void OnRep_QuickSlots();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_QuickSlots, EditAnywhere, BlueprintReadOnly, Category = "Inventory|QuickSlot")
	TArray<FInventorySlot> QuickSlots;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
	virtual bool UseItem(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
	virtual bool AutoEquipItem(int32 MainSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	virtual bool EquipToQuickSlot(int32 MainSlotIndex, int32 QuickSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool UnequipFromQuickSlot(int32 QuickSlotIndex, int32 MainSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	virtual bool UseQuickSlot(int32 QuickSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
	virtual bool DropItem(int32 SlotIndex, int32 Quantity);
	
	UFUNCTION(Server, Reliable)
	void Server_DropItem(int32 SlotIndex, int32 Quantity);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory|Drop")
	TSubclassOf<class AActor> BaseItemActorClass;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
	virtual bool LootItem(class ANCItemActor* ItemToLoot);
	
	UFUNCTION(Server, Reliable)
	void Server_LootItem(class ANCItemActor* ItemToLoot);
	
	virtual void InitializeInventory() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetQuickSlotData(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetMainSlotData(int32 SlotIndex) const;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 CurrentEquippedSlotIndex = -1;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ForceUnArm();
};