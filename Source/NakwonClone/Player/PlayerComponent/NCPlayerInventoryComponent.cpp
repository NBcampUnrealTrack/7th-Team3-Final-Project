#include "NCPlayerInventoryComponent.h"
#include "Net/UnrealNetwork.h"

UNCPlayerInventoryComponent::UNCPlayerInventoryComponent()
{
	GridSize = FIntPoint(5, 4);
}

void UNCPlayerInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UNCPlayerInventoryComponent, QuickSlots);
}

void UNCPlayerInventoryComponent::InitializeInventory()
{
	Super::InitializeInventory();
	
	QuickSlots.Init(FInventorySlot(), 4);
}

void UNCPlayerInventoryComponent::OnRep_QuickSlots()
{
	OnQuickSlotUpdated.Broadcast();
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
	
	FGameplayTag ItemTag = Items[SlotIndex].ItemTypeTag;
	
	// TODO : 소비품(Consumable) 태그인지 확인하는 로직 추가
	// if (!ItemTag.MatchesTag(NCItemType::Consumable)) return false;
	
	FString DebugMsg = FString::Printf(TEXT("아이템 사용 태그: %s"), *ItemTag.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, DebugMsg);
	
	return RemoveItem(SlotIndex, 1);
}

