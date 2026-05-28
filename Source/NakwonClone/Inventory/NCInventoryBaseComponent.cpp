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

bool UNCInventoryBaseComponent::AddItem(FGameplayTag ItemTypeTag, int32 Quantity)
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
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemTypeTag.GetTagName(), ContextString);
	
	if (!ItemData)
	{
		return false;
	}
	
	int32 RemainingQuantity = Quantity;
	
	if (ItemData->MaxStackSize > 1)
	{
		int32 StackableSlotIndex;
		while (RemainingQuantity > 0 && FindStackableSlot(ItemTypeTag, ItemData->MaxStackSize, StackableSlotIndex))
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
        
		Items[FromIndex].ItemTypeTag = FGameplayTag::EmptyTag;
		Items[FromIndex].Quantity = 0;
        
		bSuccess = true;
	}
	else
	{
		if (Items[ToIndex].ItemTypeTag == Items[FromIndex].ItemTypeTag)
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

bool UNCInventoryBaseComponent::FindStackableSlot(FGameplayTag ItemTypeTag, int32 MaxStackSize,
	int32& OutSlotIndex) const
{
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i].ItemTypeTag == ItemTypeTag && Items[i].Quantity < MaxStackSize)
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
	
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(Items[SourceIndex].ItemTypeTag.GetTagName(), TEXT("CombineItem"));
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
