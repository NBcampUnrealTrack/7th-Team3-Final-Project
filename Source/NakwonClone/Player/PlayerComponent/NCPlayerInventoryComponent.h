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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConsumableSelectionRequested);
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
	
public:
    UPROPERTY(ReplicatedUsing = OnRep_Presets, EditAnywhere, BlueprintReadOnly, Category = "Inventory|Preset")
    TArray<FEquipmentPreset> EquipmentPresets;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 CurrentEquippedPresetIndex = -1;
	
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnConsumableSelectionRequested OnConsumableSelectionRequested;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Consumable")
	int32 SelectedConsumableIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
	void SetSelectedConsumableIndex(int32 Index);

    virtual void InitializeInventory() override;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    void ApplyPreset(int32 PresetIndex);

    UFUNCTION(Server, Reliable)
    void Server_ApplyPreset(int32 PresetIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    bool EquipToPreset(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

    UFUNCTION(Server, Reliable)
    void Server_EquipToPreset(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    bool UnequipFromPreset(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_UnequipFromPreset(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool EquipToConsumable(int32 MainSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_EquipToConsumable(int32 MainSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool UnequipFromConsumable(int32 PresetIndex, int32 ConsumableSlotIndex, int32 MainSlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_UnequipFromConsumable(int32 PresetIndex, int32 ConsumableSlotIndex, int32 MainSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    void UnequipPresetToBag(int32 PresetIndex, ENCPresetCell Cell);

    UFUNCTION(Server, Reliable)
    void Server_UnequipPresetToBag(int32 PresetIndex, ENCPresetCell Cell);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    void UnequipConsumableToBag(int32 PresetIndex, int32 ConsumableSlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_UnequipConsumableToBag(int32 PresetIndex, int32 ConsumableSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Preset")
    bool MovePresetToPreset(int32 FromPresetIndex, ENCPresetCell FromCell, int32 ToPresetIndex, ENCPresetCell ToCell);

    UFUNCTION(Server, Reliable)
    void Server_MovePresetToPreset(int32 FromPresetIndex, ENCPresetCell FromCell, int32 ToPresetIndex, ENCPresetCell ToCell);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool MoveConsumableToConsumable(int32 FromPreset, int32 FromSlot, int32 ToPreset, int32 ToSlot);

    UFUNCTION(Server, Reliable)
    void Server_MoveConsumableToConsumable(int32 FromPreset, int32 FromSlot, int32 ToPreset, int32 ToSlot);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool UseConsumableSlot(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_UseConsumableSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
    virtual bool AutoEquipItem(int32 MainSlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_AutoEquipItem(int32 MainSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Action")
    virtual bool UseItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_UseItem(int32 SlotIndex);

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

    // 헌호 - 서버→모든 클라 아이템 사용 이벤트 전파
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnItemUsed(FGameplayTag ItemTag);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ForceUnArm();
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FEquipmentPreset GetPresetData(int32 PresetIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetPresetActiveWeapon(int32 PresetIndex) const;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetConsumableData(int32 PresetIndex, int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Getters")
	FInventorySlot GetMainSlotData(int32 SlotIndex) const;
	
	TArray<FEquipmentPreset> GetPresetsArray() const { return EquipmentPresets; }
	void SetPresetsArray(const TArray<FEquipmentPreset>& In) { EquipmentPresets = In; if (EquipmentPresets.Num() != 2) EquipmentPresets.Init(FEquipmentPreset(), 2); }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void TakeItemFromLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_TakeItemFromLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void TakeLootBoxItemToPreset(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

	UFUNCTION(Server, Reliable)
	void Server_TakeLootBoxItemToPreset(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void TakeLootBoxItemToConsumable(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_TakeLootBoxItemToConsumable(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void MovePresetToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

	UFUNCTION(Server, Reliable)
	void Server_MovePresetToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, ENCPresetCell Cell);

	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void MoveConsumableToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_MoveConsumableToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);

	UFUNCTION(BlueprintCallable)
	void MoveLootBoxItem(AANCLootBoxActor* LootBox, int32 FromSlotIndex, int32 ToSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_MoveLootBoxItem(AANCLootBoxActor* LootBox, int32 FromSlotIndex, int32 ToSlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|LootBox")
	void PutItemToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_PutItemToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex);
	
	FConsumableItemData PendingConsumableData;
	bool bHasPendingConsumable = false;
	int32 PendingReEquipPresetIndex = -1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|DataTable")
	TObjectPtr<UDataTable> ConsumableDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|DataTable")
	TObjectPtr<UDataTable> CreditDataTable;
	
private:
	FGameplayTag GetWeaponTypeTag(FName WeaponID) const;

	// 실제 인벤토리 변경 로직 — 항상 서버(Authority)에서만 실행됨
	bool EquipToPreset_Internal(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell);
	bool UnequipFromPreset_Internal(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex);
	bool EquipToConsumable_Internal(int32 MainSlotIndex, int32 PresetIndex, int32 ConsumableSlotIndex);
	bool UnequipFromConsumable_Internal(int32 PresetIndex, int32 ConsumableSlotIndex, int32 MainSlotIndex);
	bool MovePresetToPreset_Internal(int32 FromPresetIndex, ENCPresetCell FromCell, int32 ToPresetIndex, ENCPresetCell ToCell);
	bool MoveConsumableToConsumable_Internal(int32 FromPreset, int32 FromSlot, int32 ToPreset, int32 ToSlot);
	bool UseConsumableSlot_Internal(int32 SlotIndex);
	bool AutoEquipItem_Internal(int32 MainSlotIndex);
	bool UseItem_Internal(int32 SlotIndex);
};