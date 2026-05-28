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
        
		return true;
	}
	return false;
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
