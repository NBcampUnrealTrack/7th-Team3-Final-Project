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
        OnPresetUpdated.Broadcast();
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
    OnPresetUpdated.Broadcast();

    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
        FString::Printf(TEXT("[프리셋 %d 적용]"), PresetIndex));
}

bool UNCPlayerInventoryComponent::EquipToPreset(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell)
{
    if (GetOwner()->HasAuthority())
    {
        return EquipToPreset_Internal(MainSlotIndex, PresetIndex, Cell);
    }
    Server_EquipToPreset(MainSlotIndex, PresetIndex, Cell);
    return true; // 클라이언트: 서버에 요청 전송됨, 결과는 OnRep_Presets 등으로 반영
}

void UNCPlayerInventoryComponent::Server_EquipToPreset_Implementation(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell)
{
    EquipToPreset_Internal(MainSlotIndex, PresetIndex, Cell);
}

bool UNCPlayerInventoryComponent::EquipToPreset_Internal(int32 MainSlotIndex, int32 PresetIndex, ENCPresetCell Cell)
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

    {
        FItemData ItemData;
        const bool bGotData = GetItemDataByTag(ItemID, ItemTag, ItemData);
        const bool bHasLeftTag = bGotData && ItemData.EquipTags.HasTag(NCEquip::Hand_Left);
        const bool bIsWeapon = ItemTag.MatchesTag(NCItemTag::Weapon);

        if (bIsWeapon && WeaponType.MatchesTagExact(NCWeapon::Type_TwoHanded))
        {
            Cell = ENCPresetCell::Two;
        }
        else if (Cell == ENCPresetCell::Left && bHasLeftTag)
        {
            Cell = ENCPresetCell::Left;
        }
        else if (bIsWeapon || bHasLeftTag)
        {
            Cell = ENCPresetCell::Right;
        }
        else
        {
            return false;
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
    if (GetOwner()->HasAuthority())
    {
        return UnequipFromPreset_Internal(PresetIndex, Cell, MainSlotIndex);
    }
    Server_UnequipFromPreset(PresetIndex, Cell, MainSlotIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_UnequipFromPreset_Implementation(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex)
{
    UnequipFromPreset_Internal(PresetIndex, Cell, MainSlotIndex);
}

bool UNCPlayerInventoryComponent::UnequipFromPreset_Internal(int32 PresetIndex, ENCPresetCell Cell, int32 MainSlotIndex)
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
        (Cell == ENCPresetCell::Two)  ? P.TwoHand :
        (Cell == ENCPresetCell::Left) ? ((P.LeftHand.IsEmpty()  && !P.TwoHand.IsEmpty()) ? P.TwoHand : P.LeftHand) :
                                        ((P.RightHand.IsEmpty() && !P.TwoHand.IsEmpty()) ? P.TwoHand : P.RightHand);

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

    const bool bSourceIsTwoHand = (&Source == &P.TwoHand);
    const bool bSourceIsRight   = (&Source == &P.RightHand);
    const bool bWasActiveWeapon =
        (CurrentEquippedPresetIndex == PresetIndex) &&
        (bSourceIsTwoHand || (bSourceIsRight && P.TwoHand.IsEmpty()));

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
    if (GetOwner()->HasAuthority())
    {
        return EquipToConsumable_Internal(MainSlotIndex, ConsumableSlotIndex);
    }
    Server_EquipToConsumable(MainSlotIndex, ConsumableSlotIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_EquipToConsumable_Implementation(int32 MainSlotIndex, int32 ConsumableSlotIndex)
{
    EquipToConsumable_Internal(MainSlotIndex, ConsumableSlotIndex);
}

bool UNCPlayerInventoryComponent::EquipToConsumable_Internal(int32 MainSlotIndex, int32 ConsumableSlotIndex)
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
    if (GetOwner()->HasAuthority())
    {
        return UnequipFromConsumable_Internal(ConsumableSlotIndex, MainSlotIndex);
    }
    Server_UnequipFromConsumable(ConsumableSlotIndex, MainSlotIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_UnequipFromConsumable_Implementation(int32 ConsumableSlotIndex, int32 MainSlotIndex)
{
    UnequipFromConsumable_Internal(ConsumableSlotIndex, MainSlotIndex);
}

void UNCPlayerInventoryComponent::UnequipPresetToBag(int32 PresetIndex, ENCPresetCell Cell)
{
    if (GetOwner()->HasAuthority())
    {
        int32 EmptySlot = -1;
        if (FindEmptySlot(EmptySlot))
        {
            UnequipFromPreset_Internal(PresetIndex, Cell, EmptySlot);
        }
        return;
    }
    Server_UnequipPresetToBag(PresetIndex, Cell);
}

void UNCPlayerInventoryComponent::Server_UnequipPresetToBag_Implementation(int32 PresetIndex, ENCPresetCell Cell)
{
    int32 EmptySlot = -1;
    if (FindEmptySlot(EmptySlot))
    {
        UnequipFromPreset_Internal(PresetIndex, Cell, EmptySlot);
    }
}

void UNCPlayerInventoryComponent::UnequipConsumableToBag(int32 ConsumableSlotIndex)
{
    if (GetOwner()->HasAuthority())
    {
        int32 EmptySlot = -1;
        if (FindEmptySlot(EmptySlot))
        {
            UnequipFromConsumable_Internal(ConsumableSlotIndex, EmptySlot);
        }
        return;
    }
    Server_UnequipConsumableToBag(ConsumableSlotIndex);
}

void UNCPlayerInventoryComponent::Server_UnequipConsumableToBag_Implementation(int32 ConsumableSlotIndex)
{
    int32 EmptySlot = -1;
    if (FindEmptySlot(EmptySlot))
    {
        UnequipFromConsumable_Internal(ConsumableSlotIndex, EmptySlot);
    }
}

bool UNCPlayerInventoryComponent::UnequipFromConsumable_Internal(int32 ConsumableSlotIndex, int32 MainSlotIndex)
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

bool UNCPlayerInventoryComponent::MovePresetToPreset(int32 FromPresetIndex, ENCPresetCell FromCell, int32 ToPresetIndex, ENCPresetCell ToCell)
{
    if (GetOwner()->HasAuthority())
    {
        return MovePresetToPreset_Internal(FromPresetIndex, FromCell, ToPresetIndex, ToCell);
    }
    Server_MovePresetToPreset(FromPresetIndex, FromCell, ToPresetIndex, ToCell);
    return true;
}

void UNCPlayerInventoryComponent::Server_MovePresetToPreset_Implementation(int32 FromPresetIndex, ENCPresetCell FromCell, int32 ToPresetIndex, ENCPresetCell ToCell)
{
    MovePresetToPreset_Internal(FromPresetIndex, FromCell, ToPresetIndex, ToCell);
}

bool UNCPlayerInventoryComponent::MovePresetToPreset_Internal(int32 FromPresetIndex, ENCPresetCell FromCell, int32 ToPresetIndex, ENCPresetCell ToCell)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!EquipmentPresets.IsValidIndex(FromPresetIndex) || !EquipmentPresets.IsValidIndex(ToPresetIndex))
    {
        return false;
    }
    if (FromPresetIndex == ToPresetIndex && FromCell == ToCell)
    {
        return false;
    }

    FEquipmentPreset& PFrom = EquipmentPresets[FromPresetIndex];
    FEquipmentPreset& PTo   = EquipmentPresets[ToPresetIndex];

    auto ResolveCell = [](FEquipmentPreset& P, ENCPresetCell Cell) -> FInventorySlot&
    {
        if (Cell == ENCPresetCell::Two)  return P.TwoHand;
        if (!P.TwoHand.IsEmpty())        return P.TwoHand;
        if (Cell == ENCPresetCell::Left) return P.LeftHand;
        return P.RightHand;
    };

    FInventorySlot& Src = ResolveCell(PFrom, FromCell);
    if (Src.IsEmpty())
    {
        return false;
    }

    const FGameplayTag SrcTag = Src.ItemTypeTag;
    const FGameplayTag SrcWeapon = GetWeaponTypeTag(Src.ItemID);
    ENCPresetCell DestCell = ToCell;
    if (SrcTag.MatchesTag(NCItemTag::Weapon))
    {
        if (SrcWeapon.MatchesTagExact(NCWeapon::Type_TwoHanded))      DestCell = ENCPresetCell::Two;
        else if (SrcWeapon.MatchesTagExact(NCWeapon::Type_OneHanded)) DestCell = ENCPresetCell::Right;
    }

    FInventorySlot& Dst =
        (DestCell == ENCPresetCell::Two)  ? PTo.TwoHand  :
        (DestCell == ENCPresetCell::Left) ? PTo.LeftHand : PTo.RightHand;

    if (CurrentEquippedPresetIndex == FromPresetIndex || CurrentEquippedPresetIndex == ToPresetIndex)
    {
        if (ANCPlayerState* PS = Cast<ANCPlayerState>(GetOwner()))
            if (APawn* Pawn = PS->GetPawn())
                if (UNCCombatComponent* Combat = Pawn->FindComponentByClass<UNCCombatComponent>())
                {
                    Combat->UnEquipWeapon();
                    CurrentEquippedPresetIndex = -1;
                }
    }

    const FInventorySlot Moving = Src;
    Src = FInventorySlot();

    TArray<FInventorySlot> Displaced;
    auto Take = [&Displaced](FInventorySlot& Cell)
    {
        if (!Cell.IsEmpty()) { Displaced.Add(Cell); Cell = FInventorySlot(); }
    };

    if (DestCell == ENCPresetCell::Two)
    {
        Take(PTo.RightHand);
        Take(PTo.LeftHand);
        Take(PTo.TwoHand);
        PTo.TwoHand = Moving;
    }
    else
    {
        Take(PTo.TwoHand);
        FInventorySlot& DstRef = (DestCell == ENCPresetCell::Left) ? PTo.LeftHand : PTo.RightHand;
        Take(DstRef);
        DstRef = Moving;
    }

    auto PlaceBack = [&](const FInventorySlot& Item)
    {
        const FGameplayTag WType = GetWeaponTypeTag(Item.ItemID);
        if (WType.MatchesTagExact(NCWeapon::Type_TwoHanded))
        {
            if (PFrom.TwoHand.IsEmpty() && PFrom.RightHand.IsEmpty() && PFrom.LeftHand.IsEmpty())
            {
                PFrom.TwoHand = Item;
                return;
            }
        }
        else
        {
            if (PFrom.RightHand.IsEmpty()) { PFrom.RightHand = Item; return; }
            if (PFrom.LeftHand.IsEmpty())  { PFrom.LeftHand  = Item; return; }
        }
        for (int32 i = 0; i < Items.Num(); ++i)
            if (Items[i].IsEmpty()) { Items[i] = Item; return; }
    };

    for (const FInventorySlot& Item : Displaced)
    {
        PlaceBack(Item);
    }

    OnInventoryUpdated.Broadcast();
    OnPresetUpdated.Broadcast();
    return true;
}

bool UNCPlayerInventoryComponent::MoveConsumableToConsumable(int32 FromIndex, int32 ToIndex)
{
    if (GetOwner()->HasAuthority())
    {
        return MoveConsumableToConsumable_Internal(FromIndex, ToIndex);
    }
    Server_MoveConsumableToConsumable(FromIndex, ToIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_MoveConsumableToConsumable_Implementation(int32 FromIndex, int32 ToIndex)
{
    MoveConsumableToConsumable_Internal(FromIndex, ToIndex);
}

bool UNCPlayerInventoryComponent::MoveConsumableToConsumable_Internal(int32 FromIndex, int32 ToIndex)
{
    if (!GetOwner()->HasAuthority())
    {
        return false;
    }
    if (!ConsumableQuickSlots.IsValidIndex(FromIndex) || !ConsumableQuickSlots.IsValidIndex(ToIndex))
    {
        return false;
    }
    if (FromIndex == ToIndex || ConsumableQuickSlots[FromIndex].IsEmpty())
    {
        return false;
    }

    FInventorySlot& A = ConsumableQuickSlots[FromIndex];
    FInventorySlot& B = ConsumableQuickSlots[ToIndex];

    if (!B.IsEmpty() && B.ItemID == A.ItemID && B.ItemTypeTag == A.ItemTypeTag)
    {
        FItemData Data;
        if (GetItemDataByTag(A.ItemID, A.ItemTypeTag, Data))
        {
            const int32 Room = Data.MaxStackSize - B.Quantity;
            if (Room > 0)
            {
                const int32 MoveAmount = FMath::Min(Room, A.Quantity);
                B.Quantity += MoveAmount;
                A.Quantity -= MoveAmount;
                if (A.Quantity <= 0) A = FInventorySlot();
                OnQuickSlotUpdated.Broadcast();
                return true;
            }
        }
    }

    FInventorySlot Temp = B;
    B = A;
    A = Temp;

    OnQuickSlotUpdated.Broadcast();
    return true;
}

bool UNCPlayerInventoryComponent::UseConsumableSlot(int32 SlotIndex)
{
    if (GetOwner()->HasAuthority())
    {
        return UseConsumableSlot_Internal(SlotIndex);
    }
    Server_UseConsumableSlot(SlotIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_UseConsumableSlot_Implementation(int32 SlotIndex)
{
    UseConsumableSlot_Internal(SlotIndex);
}

bool UNCPlayerInventoryComponent::UseConsumableSlot_Internal(int32 SlotIndex)
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
    
    const int32 PresetToRestore = CurrentEquippedPresetIndex;
    if (PresetToRestore != -1)
    {
        ForceUnArm();
        PendingReEquipPresetIndex = PresetToRestore;
    }

    OnItemUsed.Broadcast(ItemTag);
    return true;
}

bool UNCPlayerInventoryComponent::AutoEquipItem(int32 MainSlotIndex)
{
    if (GetOwner()->HasAuthority())
    {
        return AutoEquipItem_Internal(MainSlotIndex);
    }
    Server_AutoEquipItem(MainSlotIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_AutoEquipItem_Implementation(int32 MainSlotIndex)
{
    AutoEquipItem_Internal(MainSlotIndex);
}

bool UNCPlayerInventoryComponent::AutoEquipItem_Internal(int32 MainSlotIndex)
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
        const FEquipmentPreset& P1 = EquipmentPresets[1];
        const bool bP0CellEmpty = (Cell == ENCPresetCell::Two) ? P0.TwoHand.IsEmpty() : P0.RightHand.IsEmpty();
        const bool bP1CellEmpty = (Cell == ENCPresetCell::Two) ? P1.TwoHand.IsEmpty() : P1.RightHand.IsEmpty();
        const bool bP0Ready = bP0CellEmpty && P0.TwoHand.IsEmpty() && (Cell != ENCPresetCell::Two || (P0.RightHand.IsEmpty() && P0.LeftHand.IsEmpty()));
        const bool bP1Ready = bP1CellEmpty && P1.TwoHand.IsEmpty() && (Cell != ENCPresetCell::Two || (P1.RightHand.IsEmpty() && P1.LeftHand.IsEmpty()));
        
        if (bP0Ready)
        {
            return EquipToPreset_Internal(MainSlotIndex, 0, Cell);
        }
        
        if (bP1Ready)
        {
            return EquipToPreset_Internal(MainSlotIndex, 1, Cell);
        }
        
        return false;
    }
    
    else if (ItemTag.MatchesTag(NCItemTag::Heal))
    {
        return EquipToConsumable_Internal(MainSlotIndex, 0);
    }
    else if (ItemTag.MatchesTag(NCItemTag::Food))
    {
        return EquipToConsumable_Internal(MainSlotIndex, 1);
    }

    return false;
}

bool UNCPlayerInventoryComponent::UseItem(int32 SlotIndex)
{
    if (GetOwner()->HasAuthority())
    {
        return UseItem_Internal(SlotIndex);
    }
    Server_UseItem(SlotIndex);
    return true;
}

void UNCPlayerInventoryComponent::Server_UseItem_Implementation(int32 SlotIndex)
{
    UseItem_Internal(SlotIndex);
}

bool UNCPlayerInventoryComponent::UseItem_Internal(int32 SlotIndex)
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

void UNCPlayerInventoryComponent::TakeLootBoxItemToPreset(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, ENCPresetCell Cell)
{
    if (!LootBox)
    {
        return;
    }
    
    Server_TakeLootBoxItemToPreset(LootBox, BoxSlotIndex, PresetIndex, Cell);
}

void UNCPlayerInventoryComponent::TakeLootBoxItemToConsumable(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 ConsumableSlotIndex)
{
    if (!LootBox)
    {
        return;
    }
    
    Server_TakeLootBoxItemToConsumable(LootBox, BoxSlotIndex, ConsumableSlotIndex);
}

void UNCPlayerInventoryComponent::MoveLootBoxItem(AANCLootBoxActor* LootBox, int32 FromSlotIndex, int32 ToSlotIndex)
{
    Server_MoveLootBoxItem(LootBox, FromSlotIndex, ToSlotIndex);
}

void UNCPlayerInventoryComponent::PutItemToLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex)
{
    Server_PutItemToLootBox(LootBox, BoxSlotIndex, PlayerSlotIndex);
}

void UNCPlayerInventoryComponent::Server_PutItemToLootBox_Implementation(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PlayerSlotIndex)
{
    if (!LootBox)
    {
        return;
    }
    UNCInventoryBaseComponent* LootInv = LootBox->GetLootInventory();
    if (!LootInv)
    {
        return;
    }
    if (!Items.IsValidIndex(PlayerSlotIndex) || Items[PlayerSlotIndex].IsEmpty())
    {
        return;
    }
    if (!LootInv->Items.IsValidIndex(BoxSlotIndex))
    {
        return;
    }
    
    FInventorySlot Temp = LootInv->Items[BoxSlotIndex];
    LootInv->Items[BoxSlotIndex] = Items[PlayerSlotIndex];
    Items[PlayerSlotIndex] = Temp;

    OnInventoryUpdated.Broadcast();
    LootInv->OnInventoryUpdated.Broadcast();
}

void UNCPlayerInventoryComponent::Server_MoveLootBoxItem_Implementation(AANCLootBoxActor* LootBox, int32 FromSlotIndex, int32 ToSlotIndex)
{
    if (!LootBox)
    {
        return;
    }

    UNCInventoryBaseComponent* LootInv = LootBox->GetLootInventory();
    if (!LootInv)
    {
        return;
    }
    if (!LootInv->Items.IsValidIndex(FromSlotIndex) || !LootInv->Items.IsValidIndex(ToSlotIndex))
    {
        return;
    }
    
    LootInv->Items.Swap(FromSlotIndex, ToSlotIndex);
    LootInv->OnInventoryUpdated.Broadcast();
}

void UNCPlayerInventoryComponent::Server_TakeLootBoxItemToPreset_Implementation(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 PresetIndex, ENCPresetCell Cell)
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
    
    int32 TempSlot = -1;
    if (!FindEmptySlot(TempSlot))
    {
        return;
    }

    if (!LootInventory->TransferItemTo(this, BoxSlotIndex, TempSlot)) return;

    EquipToPreset(TempSlot, PresetIndex, Cell);
}

void UNCPlayerInventoryComponent::Server_TakeLootBoxItemToConsumable_Implementation(AANCLootBoxActor* LootBox, int32 BoxSlotIndex, int32 ConsumableSlotIndex)
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
    
    int32 TempSlot = -1;
    if (!FindEmptySlot(TempSlot))
    {
        return;
    }
    
    if (!LootInventory->TransferItemTo(this, BoxSlotIndex, TempSlot)) return;

    EquipToConsumable(TempSlot, ConsumableSlotIndex);
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
    
    if (!Items[TargetSlot].IsEmpty())
    {
        FInventorySlot Temp = LootInventory->Items[BoxSlotIndex];
        LootInventory->Items[BoxSlotIndex] = Items[TargetSlot];
        Items[TargetSlot] = Temp;
        OnInventoryUpdated.Broadcast();
        LootInventory->OnInventoryUpdated.Broadcast();
    }
    else
    {
        LootInventory->TransferItemTo(this, BoxSlotIndex, TargetSlot);
    }
}

// 헌호 - 서버에서 호출 → 모든 클라에서 OnItemUsed 델리게이트 실행
void UNCPlayerInventoryComponent::Multicast_OnItemUsed_Implementation(FGameplayTag ItemTag)
{
    OnItemUsed.Broadcast(ItemTag);
}