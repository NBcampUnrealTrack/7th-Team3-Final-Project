#include "NCInventoryBaseComponent.h"
#include "Net/UnrealNetwork.h"

UNCInventoryBaseComponent::UNCInventoryBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GridSize = FIntPoint(5,4);
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
	
	if (Quantity <= 0)
	{
		return false;
	}
	
	if (!ItemDataTable)
	{
		return false;
	}
	
	FString ContextString = TEXT("Item Add");
	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemTypeTag.GetTagName(), ContextString);
	
	if (!ItemData)
	{
		return false;
	}
	
	// todo : 아이템 스택 로직 구현
	
	int32 EmptySlotIndex;
	if (FindEmptySlot(EmptySlotIndex))
	{
		Items[EmptySlotIndex].ItemTypeTag = ItemTypeTag;
		Items[EmptySlotIndex].Quantity = Quantity;
		
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
