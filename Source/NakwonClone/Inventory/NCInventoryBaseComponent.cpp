#include "NCInventoryBaseComponent.h"
#include "Net/UnrealNetwork.h"

UNCInventoryBaseComponent::UNCInventoryBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GridSize = FIntPoint(5,4);
	
	SetIsReplicatedByDefault(true);
}


void UNCInventoryBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		InitializeInventory();
	}
}

void UNCInventoryBaseComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UNCInventoryBaseComponent, Items);
}

void UNCInventoryBaseComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}

void UNCInventoryBaseComponent::InitializeInventory()
{
	int32 TotalSlots = GridSize.X * GridSize.Y;	
	
	Items.Init(FInventorySlot(), TotalSlots);
}

bool UNCInventoryBaseComponent::GetItemDataByTag(FName ItemID, FGameplayTag ItemTag, FItemData& OutItemData) const
{
	if (!ItemDataTable || !ItemTag.IsValid())
	{
		return false;
	}
	
	FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("GetItemData_Helper"));
    
	if (FoundData)
	{
		OutItemData = *FoundData;
		return true;
	}

	return false;
}

bool UNCInventoryBaseComponent::AddItem(FName ItemID, FGameplayTag ItemTypeTag, int32 Quantity)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}
	
	if (Quantity <= 0 || !ItemDataTable || !ItemTypeTag.IsValid())
	{
		return false;
	}
	
	FString ContextString = TEXT("Item Add");
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, ContextString);
	
	if (!ItemData)
	{
		return false;
	}
	
	int32 RemainingQuantity = Quantity;
	
	if (ItemData->MaxStackSize > 1)
	{
		int32 StackableSlotIndex;
		
		while (RemainingQuantity > 0 && FindStackableSlot(ItemID, ItemTypeTag, ItemData->MaxStackSize, StackableSlotIndex))
		{
			int32 RoomInSlot = ItemData->MaxStackSize - Items[StackableSlotIndex].Quantity;

			if (RemainingQuantity <= RoomInSlot)
			{
				Items[StackableSlotIndex].Quantity += RemainingQuantity;
				RemainingQuantity = 0;
			}
			else
			{
				Items[StackableSlotIndex].Quantity = ItemData->MaxStackSize;
				RemainingQuantity -= RoomInSlot;
			}
		}
	}
	
	while (RemainingQuantity > 0)
	{
		int32 EmptySlotIndex;
		if (FindEmptySlot(EmptySlotIndex))
		{
			Items[EmptySlotIndex].ItemID = ItemID;
			Items[EmptySlotIndex].ItemTypeTag = ItemTypeTag;

			if (RemainingQuantity <= ItemData->MaxStackSize)
			{
				Items[EmptySlotIndex].Quantity = RemainingQuantity;
				RemainingQuantity = 0;
			}
			else
			{
				Items[EmptySlotIndex].Quantity = ItemData->MaxStackSize;
				RemainingQuantity -= ItemData->MaxStackSize;
			}
		}
		else
		{
			break;
		}
	}

	if (RemainingQuantity != Quantity)
	{
		OnInventoryUpdated.Broadcast();
	}
	
	return RemainingQuantity == 0;
}

bool UNCInventoryBaseComponent::RemoveItem(int32 SlotIndex, int32 Quantity)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}
	if (Quantity <= 0 || !Items.IsValidIndex(SlotIndex) || Items[SlotIndex].IsEmpty())
	{
		return false;
	}

	if (Items[SlotIndex].Quantity >= Quantity)
	{
		Items[SlotIndex].Quantity -= Quantity;

		if (Items[SlotIndex].Quantity == 0)
		{
			Items[SlotIndex].ItemID = NAME_None;
			Items[SlotIndex].ItemTypeTag = FGameplayTag::EmptyTag;
		}
        
		OnInventoryUpdated.Broadcast();
		
		return true;
	}
	return false;
}

bool UNCInventoryBaseComponent::MoveItem(int32 FromIndex, int32 ToIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}
	
	if (FromIndex == ToIndex || !Items.IsValidIndex(FromIndex) || !Items.IsValidIndex(ToIndex) || Items[FromIndex].IsEmpty())
	{
		return false;
	}
	
	bool bSuccess = false;
	
	if (Items[ToIndex].IsEmpty())
	{
		Items[ToIndex] = Items[FromIndex];
        
		Items[FromIndex].ItemID = NAME_None;
		Items[FromIndex].ItemTypeTag = FGameplayTag::EmptyTag;
		Items[FromIndex].Quantity = 0;
        
		bSuccess = true;
	}
	else
	{
		if (Items[ToIndex].ItemID == Items[FromIndex].ItemID && Items[ToIndex].ItemTypeTag == Items[FromIndex].ItemTypeTag)
		{
			bSuccess = CombineSlots(FromIndex, ToIndex);
		}
		else
		{
			bSuccess = SwapSlots(ToIndex, FromIndex);
		}
	}
	
	if (bSuccess)
	{
		OnInventoryUpdated.Broadcast();
	}

	return bSuccess;
}

bool UNCInventoryBaseComponent::TransferItemTo(UNCInventoryBaseComponent* TargetInventory, int32 FromIndex,
	int32 ToIndex)
{
    if (!GetOwner()->HasAuthority() || !TargetInventory)
    {
	    return false;
    }
	
    if (this == TargetInventory)
    {
        return MoveItem(FromIndex, ToIndex);
    }

    if (!Items.IsValidIndex(FromIndex) || Items[FromIndex].IsEmpty())
    {
	    return false;
    }
	
    if (!TargetInventory->Items.IsValidIndex(ToIndex))
    {
	    return false;
    }
	
    bool bSuccess = false;

    if (TargetInventory->Items[ToIndex].IsEmpty())
    {
        TargetInventory->Items[ToIndex] = Items[FromIndex];
        
        Items[FromIndex].ItemID = NAME_None;
        Items[FromIndex].ItemTypeTag = FGameplayTag::EmptyTag;
        Items[FromIndex].Quantity = 0;
        
        bSuccess = true;
    }
    else
    {
        bool bIsSameItem = (TargetInventory->Items[ToIndex].ItemID == Items[FromIndex].ItemID) && 
                           (TargetInventory->Items[ToIndex].ItemTypeTag == Items[FromIndex].ItemTypeTag);

        if (bIsSameItem)
        {
            FItemData ItemData;
            if (GetItemDataByTag(Items[FromIndex].ItemID, Items[FromIndex].ItemTypeTag, ItemData)) 
            {
                int32 RoomInTarget = ItemData.MaxStackSize - TargetInventory->Items[ToIndex].Quantity;

                if (RoomInTarget > 0)
                {
                    if (Items[FromIndex].Quantity <= RoomInTarget)
                    {
                        TargetInventory->Items[ToIndex].Quantity += Items[FromIndex].Quantity;
                        
                        Items[FromIndex].ItemID = NAME_None;
                        Items[FromIndex].ItemTypeTag = FGameplayTag::EmptyTag;
                        Items[FromIndex].Quantity = 0;
                    }
                    else
                    {
                        TargetInventory->Items[ToIndex].Quantity = ItemData.MaxStackSize;
                        Items[FromIndex].Quantity -= RoomInTarget;
                    }
                    bSuccess = true;
                }
                else
                {
                    FInventorySlot TempSlot = TargetInventory->Items[ToIndex];
                    TargetInventory->Items[ToIndex] = Items[FromIndex];
                    Items[FromIndex] = TempSlot;
                    
                    bSuccess = true;
                }
            }
        }
        else
        {
            FInventorySlot TempSlot = TargetInventory->Items[ToIndex];
            TargetInventory->Items[ToIndex] = Items[FromIndex];
            Items[FromIndex] = TempSlot;
            
            bSuccess = true;
        }
    }

    if (bSuccess)
    {
        this->OnInventoryUpdated.Broadcast();
        TargetInventory->OnInventoryUpdated.Broadcast();
    }

    return bSuccess;
}

bool UNCInventoryBaseComponent::FindEmptySlot(int32& OutSlotIndex) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].IsEmpty())
		{
			OutSlotIndex = i;
			return true;
		}
	}
	return false;
}

bool UNCInventoryBaseComponent::FindStackableSlot(FName ItemID, FGameplayTag ItemTypeTag, int32 MaxStackSize, int32& OutSlotIndex) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].ItemID == ItemID && Items[i].ItemTypeTag == ItemTypeTag && Items[i].Quantity < MaxStackSize)
		{
			OutSlotIndex = i;
			return true;
		}
	}
	return false;
}

bool UNCInventoryBaseComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
	FInventorySlot TempSlot = Items[IndexA];
	Items[IndexA] = Items[IndexB];
	Items[IndexB] = TempSlot;
    
	return true;
}

bool UNCInventoryBaseComponent::CombineSlots(int32 SourceIndex, int32 TargetIndex)
{
	if (!ItemDataTable)
	{
		return false;
	}
	
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(Items[SourceIndex].ItemID, TEXT("CombineItem"));
	if (!ItemData)
	{
		return false;
	}
	
	int32 RoomInTarget = ItemData->MaxStackSize - Items[TargetIndex].Quantity;
	if (RoomInTarget <= 0)
	{
		return false;
	}
	
	if (Items[SourceIndex].Quantity <= RoomInTarget)
	{
		Items[TargetIndex].Quantity += Items[SourceIndex].Quantity;
		
		Items[SourceIndex].ItemID = NAME_None;
		Items[SourceIndex].ItemTypeTag = FGameplayTag::EmptyTag;
		Items[SourceIndex].Quantity = 0;
	}

	else
	{
		Items[TargetIndex].Quantity = ItemData->MaxStackSize;
		Items[SourceIndex].Quantity -= RoomInTarget;
	}
	
	return true;
}
