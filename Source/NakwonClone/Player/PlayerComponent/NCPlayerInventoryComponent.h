#pragma once

#include "CoreMinimal.h"
#include "Inventory/NCInventoryBaseComponent.h"
#include "Inventory/NCInventoryType.h"

#include "NCPlayerInventoryComponent.generated.h"

class AANCLootBoxActor;

UENUM(BlueprintType)
enum class ENCPresetCell : uint8
{
	Right,   
	Left,    
	Two     
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPresetUpdated);
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
	FOnPresetUpdated OnPresetUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemUsedSignature OnItemUsed;
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    
	UFUNCTION()
	void OnRep_Presets();
	
	UFUNCTION()
	virtual void OnRep_QuickSlots();
	
public:
    UPROPERTY(ReplicatedUsing = OnRep_Presets, EditAnywhere, BlueprintReadOnly, Category = "Inventory|Preset")
    TArray<FEquipmentPreset> EquipmentPresets;

    UPROPERTY(ReplicatedUsing = OnRep_QuickSlots, EditAnywhere, BlueprintReadOnly, Category = "Inventory|QuickSlot")
    TArray<FInventorySlot> ConsumableQuickSlots;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 CurrentEquippedPresetIndex = -1;

    virtual void InitializeInventory() override;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    void ApplyPreset(int32 PresetIndex);

    UFUNCTION(Server, Reliable)
    void Server_ApplyPreset(int32 PresetIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    bool EquipToPreset(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    bool UnequipFromPreset(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool EquipToConsumable(int32 MainSlotIndex, int32 ConsumableSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool UnequipFromConsumable(int32 ConsumableSlotIndex, int32 MainSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool UseConsumableSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
    virtual bool AutoEquipItem(int32 MainSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
    virtual bool UseItem(int32 SlotIndex);

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

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ForceUnArm();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FEquipmentPreset GetPresetData(int32 PresetIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetPresetActiveWeapon(int32 PresetIndex) const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetConsumableData(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetMainSlotData(int32 SlotIndex) const;
	
	TArray<FEquipmentPreset> GetPresetsArray() const { return EquipmentPresets; }
	void SetPresetsArray(const TArray<FEquipmentPreset>& In) { EquipmentPresets = In; }
	TArray<FInventorySlot> GetConsumableArray() const { return ConsumableQuickSlots; }
	void SetConsumableArray(const TArray<FInventorySlot>& In) { ConsumableQuickSlots = In; }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void TakeItemFromLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_TakeItemFromLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex);
	
	FConsumableItemData PendingConsumableData;
	bool bHasPendingConsumable = false;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|DataTable")
	TObjectPtr<UDataTable> ConsumableDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|DataTable")
	TObjectPtr<UDataTable> CreditDataTable;
	
private:
	FGameplayTag GetWeaponTypeTag(FName WeaponID) const;
};