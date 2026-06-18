#include "NCPlayerInventoryComponent.h"

#include "Framwork/PlayerState/NCPlayerState.h"
#include "Framwork/GameInstacne/NCGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Item/NCItemActor.h"
#include "Player/PlayerAnimation/NCCombatComponent.h"
#include "Player/PlayerCharacter/NCBaseCharacter.h"
#include "Item/ANCLootBoxActor.h"	
#include "Item/NCItemActor.h"

UNCPlayerInventoryComponent::UNCPlayerInventoryComponent()
{
	GridSize = FIntPoint(5, 4);
}

void UNCPlayerInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNCPlayerInventoryComponent, EquipmentPresets);
	DOREPLIFETIME(UNCPlayerInventoryComponent, ConsumableQuickSlots);
}

void UNCPlayerInventoryComponent::SetSelectedConsumableIndex(int32 Index)
{
    if (ConsumableQuickSlots.IsValidIndex(Index))
    {
        SelectedConsumableIndex = Index;
        OnQuickSlotUpdated.Broadcast();
    }
}

void UNCPlayerInventoryComponent::InitializeInventory()
{
	Super::InitializeInventory();

	EquipmentPresets.Init(FEquipmentPreset(), 2);
	ConsumableQuickSlots.Init(FInventorySlot(), 6);
}

void UNCPlayerInventoryComponent::OnRep_Presets()
{
	OnPresetUpdated.Broadcast();
}
void UNCPlayerInventoryComponent::OnRep_QuickSlots()
{
    OnQuickSlotUpdated.Broadcast();
}

FGameplayTag UNCPlayerInventoryComponent::GetWeaponTypeTag(FName WeaponID) const
{
    if (UWorld* World = GetWorld())
    {
        if (UNCGameInstance* GI = Cast<UNCGameInstance>(World->GetGameInstance()))
        {
            if (FNCWeaponData* Data = GI->GetWeaponData(WeaponID))
            {
                return Data->WeaponTypeTag;
            }
        }
    }
    return FGameplayTag::EmptyTag;
}

FEquipmentPreset UNCPlayerInventoryComponent::GetPresetData(int32 PresetIndex) const
{
    return EquipmentPresets.IsValidIndex(PresetIndex) ? EquipmentPresets[PresetIndex] : FEquipmentPreset();
}

FInventorySlot UNCPlayerInventoryComponent::GetPresetActiveWeapon(int32 PresetIndex) const
{
    if (!EquipmentPresets.IsValidIndex(PresetIndex))
        return FInventorySlot();

    const FEquipmentPreset& P = EquipmentPresets[PresetIndex];
    return P.IsTwoHandActive() ? P.TwoHand : P.RightHand;
}

FInventorySlot UNCPlayerInventoryComponent::GetConsumableData(int32 SlotIndex) const
{
    return ConsumableQuickSlots.IsValidIndex(SlotIndex) ? ConsumableQuickSlots[SlotIndex] : FInventorySlot();
}

FInventorySlot UNCPlayerInventoryComponent::GetMainSlotData(int32 SlotIndex) const
{
    return Items.IsValidIndex(SlotIndex) ? Items[SlotIndex] : FInventorySlot();
}

void UNCPlayerInventoryComponent::ForceUnArm()
{
    if (CurrentEquippedPresetIndex == -1)
    {
        return;
    }

    if (ANCPlayerState* NCPS = Cast<ANCPlayerState>(GetOwner()))
    {
        if (APawn* NCPawn = NCPS->GetPawn())
        {
            if (UNCCombatComponent* NCCombatComp = NCPawn->FindComponentByClass<UNCCombatComponent>())
            {
                NCCombatComp->UnEquipWeapon();
                CurrentEquippedPresetIndex = -1;

                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, TEXT("[H키 : 맨손 전환]"));
            }
        }
    }
}

void UNCPlayerInventoryComponent::ApplyPreset(int32 PresetIndex)
{
    Server_ApplyPreset(PresetIndex);
}

void UNCPlayerInventoryComponent::Server_ApplyPreset_Implementation(int32 PresetIndex)
{
    if (!GetOwner()->HasAuthority() || !EquipmentPresets.IsValidIndex(PresetIndex))
    {
        return;
    }

    ANCPlayerState* PS = Cast<ANCPlayerState>(GetOwner());
    APawn* Pawn = PS ? PS->GetPawn() : nullptr;
    UNCCombatComponent* Combat = Pawn ? Pawn->FindComponentByClass<UNCCombatComponent>() : nullptr;
    if (!Combat)
    {
        return;
    }

    const FEquipmentPreset& Preset = EquipmentPresets[PresetIndex];

    if (CurrentEquippedPresetIndex == PresetIndex)
    {
        Combat->UnEquipWeapon();
        CurrentEquippedPresetIndex = -1;
        return;
    }

    Combat->UnEquipWeapon();

    auto EnsureInstance = [](FInventorySlot Slot)
    {
        FNCWeaponInstance W = Slot.WeaponInstance;
        if (W.WeaponID.IsNone())
        {
            W.WeaponID = Slot.ItemID;
            W.UniqueID = FGuid::NewGuid();
            W.CurrentDurability = 100.0f;
            W.bIsBroken = false;
        }
        return W;
    };

    if (Preset.IsTwoHandActive())
    {
        Combat->EquipWeapon(EnsureInstance(Preset.TwoHand));
    }
    else if (!Preset.RightHand.IsEmpty())
    {
        Combat->EquipWeapon(EnsureInstance(Preset.RightHand));
    }
    // TODO(왼손): if (!Preset.LeftHand.IsEmpty()) Combat->EquipOffHand(Preset.LeftHand.ItemID);

    CurrentEquippedPresetIndex = PresetIndex;

    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
        FString::Printf(TEXT("[프리셋 %d 적용]"), PresetIndex));
}

bool UNCPlayerInventoryComponent::EquipToPreset(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!Items.IsValidIndex(MainSlotIndex) || Items[MainSlotIndex].IsEmpty() || !EquipmentPresets.IsValidIndex(PresetIndex))
    {
        return false;
    }

    const FGameplayTag ItemTag = Items[MainSlotIndex].ItemTypeTag;
    const FName ItemID = Items[MainSlotIndex].ItemID;
    const FGameplayTag WeaponType = GetWeaponTypeTag(ItemID);

    if (Cell == ENCPresetCell::Right)
    {
        if (!ItemTag.MatchesTag(NCItemTag::Weapon)) return false;
        if (WeaponType.MatchesTagExact(NCWeapon::Type_TwoHanded))
            Cell = ENCPresetCell::Two;
        else if (!WeaponType.MatchesTagExact(NCWeapon::Type_OneHanded))
            return false;
    }
    else if (Cell == ENCPresetCell::Two)
    {
        if (!ItemTag.MatchesTag(NCItemTag::Weapon) || !WeaponType.MatchesTagExact(NCWeapon::Type_TwoHanded))
            return false;
    }
    else
    {
        if (ItemTag.MatchesTag(NCItemTag::Weapon) && WeaponType.MatchesTagExact(NCWeapon::Type_TwoHanded))
        {
            Cell = ENCPresetCell::Two;
        }
        else
        {
            FItemData ItemData;
            if (!GetItemDataByTag(ItemID, ItemTag, ItemData)) return false;
            if (!ItemData.EquipTags.HasTag(NCEquip::Hand_Left)) return false;
        }
    }

    FEquipmentPreset& P = EquipmentPresets[PresetIndex];

    auto ReturnCell = [&](FInventorySlot& C) -> bool
    {
        if (C.IsEmpty()) return true;
        for (int32 i = 0; i < Items.Num(); ++i)
        {
            if (Items[i].IsEmpty())
            {
                Items[i] = C;
                C = FInventorySlot();
                return true;
            }
        }
        return false;
    };

    if (Cell == ENCPresetCell::Two)
    {
        if (!ReturnCell(P.RightHand) || !ReturnCell(P.LeftHand)) return false;
    }
    else
    {
        if (!ReturnCell(P.TwoHand)) return false;
    }

    FInventorySlot& Target =
        (Cell == ENCPresetCell::Right) ? P.RightHand :
        (Cell == ENCPresetCell::Two)   ? P.TwoHand   : P.LeftHand;

    FInventorySlot Temp = Target;
    Target = Items[MainSlotIndex];
    Items[MainSlotIndex] = Temp;

    OnInventoryUpdated.Broadcast();
    OnPresetUpdated.Broadcast();
    return true;
}

bool UNCPlayerInventoryComponent::UnequipFromPreset(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!EquipmentPresets.IsValidIndex(PresetIndex) || !Items.IsValidIndex(MainSlotIndex))
    {
        return false;
    }

    FEquipmentPreset& P = EquipmentPresets[PresetIndex];
    FInventorySlot& Source =
        (Cell == ENCPresetCell::Right) ? P.RightHand :
        (Cell == ENCPresetCell::Two)   ? P.TwoHand   : P.LeftHand;

    if (Source.IsEmpty())
    {
        return false;
    }

    if (!Items[MainSlotIndex].IsEmpty())
    {
        const FGameplayTag InTag = Items[MainSlotIndex].ItemTypeTag;
        const FGameplayTag InWeapon = GetWeaponTypeTag(Items[MainSlotIndex].ItemID);
        if (Cell == ENCPresetCell::Right && (!InTag.MatchesTag(NCItemTag::Weapon) || !InWeapon.MatchesTagExact(NCWeapon::Type_OneHanded)))
        {
            return false;
        }
        if (Cell == ENCPresetCell::Two && (!InTag.MatchesTag(NCItemTag::Weapon) || !InWeapon.MatchesTagExact(NCWeapon::Type_TwoHanded)))
        {
            return false;
        }
        if (Cell == ENCPresetCell::Left)
        {
            FItemData InData;
            if (!GetItemDataByTag(Items[MainSlotIndex].ItemID, InTag, InData) ||
                !InData.EquipTags.HasTag(NCEquip::Hand_Left))
                return false;
        }
    }

    const bool bWasActiveWeapon =
        (CurrentEquippedPresetIndex == PresetIndex) &&
        ((Cell == ENCPresetCell::Two) || (Cell == ENCPresetCell::Right && P.TwoHand.IsEmpty()));

    if (bWasActiveWeapon)
    {
        if (ANCPlayerState* PS = Cast<ANCPlayerState>(GetOwner()))
        {
            if (APawn* Pawn = PS->GetPawn())
            {
                if (UNCCombatComponent* Combat = Pawn->FindComponentByClass<UNCCombatComponent>())
                {
                    Combat->UnEquipWeapon();
                    CurrentEquippedPresetIndex = -1;
                }
            }
        }
    }

    FInventorySlot Temp = Source;
    Source = Items[MainSlotIndex];
    Items[MainSlotIndex] = Temp;

    OnInventoryUpdated.Broadcast();
    OnPresetUpdated.Broadcast();
    return true;
}

bool UNCPlayerInventoryComponent::EquipToConsumable(int32 MainSlotIndex, int32 ConsumableSlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!Items.IsValidIndex(MainSlotIndex) || Items[MainSlotIndex].IsEmpty() || !ConsumableQuickSlots.IsValidIndex(ConsumableSlotIndex))
    {
        return false;
    }

    const FGameplayTag ItemTag = Items[MainSlotIndex].ItemTypeTag;
    const FName ItemID = Items[MainSlotIndex].ItemID;
    
    if (!ItemTag.MatchesTag(NCItemType::Consumable)) return false;
    if (!Items[MainSlotIndex].ItemTypeTag.MatchesTag(NCItemType::Consumable)) return false;
    
    FInventorySlot& Slot = ConsumableQuickSlots[ConsumableSlotIndex];

    if (!Slot.IsEmpty() && Slot.ItemID == ItemID && Slot.ItemTypeTag == ItemTag)
    {
        FItemData ItemData;
        if (GetItemDataByTag(ItemID, ItemTag, ItemData))
        {
            int32 Room = ItemData.MaxStackSize - Slot.Quantity;
            if (Room > 0)
            {
                if (Items[MainSlotIndex].Quantity <= Room)
                {
                    Slot.Quantity += Items[MainSlotIndex].Quantity;
                    Items[MainSlotIndex] = FInventorySlot();
                }
                else
                {
                    Slot.Quantity = ItemData.MaxStackSize;
                    Items[MainSlotIndex].Quantity -= Room;
                }
                OnInventoryUpdated.Broadcast();
                OnQuickSlotUpdated.Broadcast();
                return true;
            }
        }
    }

    // 스왑
    FInventorySlot Temp = Slot;
    Slot = Items[MainSlotIndex];
    Items[MainSlotIndex] = Temp;

    OnInventoryUpdated.Broadcast();
    OnQuickSlotUpdated.Broadcast();
    return true;
}

bool UNCPlayerInventoryComponent::UnequipFromConsumable(int32 ConsumableSlotIndex, int32 MainSlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!ConsumableQuickSlots.IsValidIndex(ConsumableSlotIndex) || !Items.IsValidIndex(MainSlotIndex))
    {
        return false;
    }
    if (ConsumableQuickSlots[ConsumableSlotIndex].IsEmpty())
    {
        return false;
    }

    if (!Items[MainSlotIndex].IsEmpty())
    {
        if (!Items[MainSlotIndex].ItemTypeTag.MatchesTag(NCItemType::Consumable)) return false;
    }

    FInventorySlot Temp = ConsumableQuickSlots[ConsumableSlotIndex];
    ConsumableQuickSlots[ConsumableSlotIndex] = Items[MainSlotIndex];
    Items[MainSlotIndex] = Temp;

    OnInventoryUpdated.Broadcast();
    OnQuickSlotUpdated.Broadcast();
    return true;
}

bool UNCPlayerInventoryComponent::UseConsumableSlot(int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!ConsumableQuickSlots.IsValidIndex(SlotIndex) || ConsumableQuickSlots[SlotIndex].IsEmpty())
    {
        return false;
    }

    const FGameplayTag ItemTag = ConsumableQuickSlots[SlotIndex].ItemTypeTag;
    const FName ItemID = ConsumableQuickSlots[SlotIndex].ItemID;

    if (!ItemTag.MatchesTag(NCItemType::Consumable))
    {
        return false;
    }

    ConsumableQuickSlots[SlotIndex].Quantity -= 1;
    if (ConsumableQuickSlots[SlotIndex].Quantity <= 0)
    {
        ConsumableQuickSlots[SlotIndex] = FInventorySlot();
    }
    OnQuickSlotUpdated.Broadcast();

    if (ItemTag.MatchesTag(NCItemTag::Heal) || ItemTag.MatchesTag(NCItemTag::Food))
    {
        if (ConsumableDataTable)
        {
            if (FConsumableItemData* Data = ConsumableDataTable->FindRow<FConsumableItemData>(ItemID, TEXT("UseConsumableSlot")))
            {
                PendingConsumableData = *Data;
                bHasPendingConsumable = true;
            }
        }
    }

    OnItemUsed.Broadcast(ItemTag);
    return true;
}

bool UNCPlayerInventoryComponent::AutoEquipItem(int32 MainSlotIndex)
{
     if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!Items.IsValidIndex(MainSlotIndex) || Items[MainSlotIndex].IsEmpty())
    {
        return false;
    }

    const FGameplayTag ItemTag = Items[MainSlotIndex].ItemTypeTag;

    if (ItemTag.MatchesTag(NCItemTag::Weapon))
    {
        const FGameplayTag WeaponType = GetWeaponTypeTag(Items[MainSlotIndex].ItemID);
        const ENCPresetCell Cell = WeaponType.MatchesTagExact(NCWeapon::Type_TwoHanded) ? ENCPresetCell::Two : ENCPresetCell::Right;

        const FEquipmentPreset& P0 = EquipmentPresets[0];
        const bool bP0CellEmpty = (Cell == ENCPresetCell::Two) ? P0.TwoHand.IsEmpty() : P0.RightHand.IsEmpty();
        return EquipToPreset(MainSlotIndex, bP0CellEmpty ? 0 : 1, Cell);
    }
    else if (ItemTag.MatchesTag(NCItemTag::Heal))
    {
        return EquipToConsumable(MainSlotIndex, 0);
    }
    else if (ItemTag.MatchesTag(NCItemTag::Food))
    {
        return EquipToConsumable(MainSlotIndex, 1);
    }

    return false;
}

bool UNCPlayerInventoryComponent::UseItem(int32 SlotIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!Items.IsValidIndex(SlotIndex) || Items[SlotIndex].IsEmpty())
    {
        return false;
    }

    const FGameplayTag ItemTag = Items[SlotIndex].ItemTypeTag;
    if (!ItemTag.MatchesTag(NCItemType::Consumable))
    {
        return false;
    }

    return RemoveItem(SlotIndex, 1);
}

bool UNCPlayerInventoryComponent::DropItem(int32 SlotIndex, int32 Quantity)
{
    Server_DropItem(SlotIndex, Quantity);
    return true;
}

void UNCPlayerInventoryComponent::Server_DropItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    if (!Items.IsValidIndex(SlotIndex) || Items[SlotIndex].IsEmpty() || Quantity <= 0)
    {
        return;
    }

    FName DropItemID = Items[SlotIndex].ItemID;
    FGameplayTag ItemTag = Items[SlotIndex].ItemTypeTag;
    int32 DropQuantity = FMath::Min(Quantity, Items[SlotIndex].Quantity);

    ANCPlayerState* OwningPlayerState = Cast<ANCPlayerState>(GetOwner());
    if (!OwningPlayerState) return;

    AActor* OwnerActor = OwningPlayerState->GetPawn();
    if (!OwnerActor) return;

    FVector SpawnLocation = OwnerActor->GetActorLocation() + (OwnerActor->GetActorForwardVector() * 100.0f);
    SpawnLocation.Z -= 20.0f;
    FRotator SpawnRotation = OwnerActor->GetActorRotation();

    if (BaseItemActorClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* DroppedItem = GetWorld()->SpawnActor<AActor>(BaseItemActorClass, SpawnLocation, SpawnRotation, SpawnParams);

        ANCItemActor* SpawnedItemActor = Cast<ANCItemActor>(DroppedItem);
        if (SpawnedItemActor)
        {
            UStaticMesh* MeshToSet = nullptr;
            UDataTable* LoadedItemDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/NakwonClone/Blueprints/Item/ItemData/DT_ItemTypeData.DT_ItemTypeData"));

            if (LoadedItemDataTable && !DropItemID.IsNone())
            {
                FItemData* FoundData = LoadedItemDataTable->FindRow<FItemData>(DropItemID, TEXT("DropItemLookup"));
                if (FoundData)
                {
                    MeshToSet = FoundData->ItemMesh;
                }
            }
            SpawnedItemActor->InitializeItemData(DropItemID, ItemTag, DropQuantity, MeshToSet);
        }
    }

    RemoveItem(SlotIndex, DropQuantity);
}

bool UNCPlayerInventoryComponent::LootItem(class ANCItemActor* ItemToLoot)
{
    Server_LootItem(ItemToLoot);
    return true;
}

void UNCPlayerInventoryComponent::Server_LootItem_Implementation(class ANCItemActor* ItemToLoot)
{
  if (!GetOwner()->HasAuthority() || !ItemToLoot)
    {
        return;
    }

    FName LootID = ItemToLoot->ItemID;
    FGameplayTag LootTag = ItemToLoot->ItemTypeTag;
    int32 LootQuantity = ItemToLoot->Quantity;

    if (LootTag.MatchesTag(NCItemTag::Credit))
    {
        if (CreditDataTable)
        {
            if (FCreditItemData* Data = CreditDataTable->FindRow<FCreditItemData>(LootID, TEXT("LootCredit")))
            {
                int32 RandomCredits = FMath::RandRange(Data->MinValue, Data->MaxValue);
                if (ANCPlayerState* NCPS = Cast<ANCPlayerState>(GetOwner()))
                {
                    NCPS->AddCredits(RandomCredits);
                }
            }
        }
        ItemToLoot->Destroy();
        return;
    }

    bool bAdded = AddItem(LootID, LootTag, LootQuantity);
    if (bAdded)
    {
        ItemToLoot->Destroy();
    }
}

void UNCPlayerInventoryComponent::TakeItemFromLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex)
{
    if (!LootBox)
    {
        return;
    }
    Server_TakeItemFromLootBox(LootBox, BoxSlotIndex, PlayerSlotIndex);
}

void UNCPlayerInventoryComponent::Server_TakeItemFromLootBox_Implementation(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex)
{
    if (!LootBox)
    {
        return;
    }
    UNCInventoryBaseComponent* LootInventory = LootBox->GetLootInventory();
    if (!LootInventory)
    {
        return;
    }
    int32 TargetSlot = PlayerSlotIndex;
    if (TargetSlot == -1)
    {
        for (int32 i = 0; i < Items.Num(); ++i)
        {
            if (Items[i].IsEmpty())
            {
                TargetSlot = i;
                break;
            }
        }
    }
    if (TargetSlot == -1)
    {
        return;
    }

    LootInventory->TransferItemTo(this, BoxSlotIndex, TargetSlot);
}

// 헌호 - 서버에서 호출 → 모든 클라에서 OnItemUsed 델리게이트 실행
void UNCPlayerInventoryComponent::Multicast_OnItemUsed_Implementation(FGameplayTag ItemTag)
{
    OnItemUsed.Broadcast(ItemTag);
}