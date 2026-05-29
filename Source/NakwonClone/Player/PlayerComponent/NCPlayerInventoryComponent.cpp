#include "NCPlayerInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "NakwonClone/Item/NCItemActor.h"

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

bool UNCPlayerInventoryComponent::EquipToQuickSlot(int32 MainSlotIndex, int32 QuickSlotIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}
	
	if (!Items.IsValidIndex(MainSlotIndex) || Items[MainSlotIndex].IsEmpty() || !QuickSlots.IsValidIndex(QuickSlotIndex))
	{
		return false;
	}
	
	FGameplayTag ItemTag = Items[MainSlotIndex].ItemTypeTag;
	
	if (QuickSlotIndex == 0 || QuickSlotIndex == 1)
	{
		if (!ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("ItemType.Equipment")))) return false;
	}
	
	else if (QuickSlotIndex == 2 || QuickSlotIndex == 3)
	{
		if (!ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("ItemType.Consumable")))) return false;
	}
	
	FInventorySlot TempSlot = QuickSlots[QuickSlotIndex];
	QuickSlots[QuickSlotIndex] = Items[MainSlotIndex];
	Items[MainSlotIndex] = TempSlot;
	
	OnInventoryUpdated.Broadcast();
	OnQuickSlotUpdated.Broadcast();
	
	return true;
}

bool UNCPlayerInventoryComponent::UseQuickSlot(int32 QuickSlotIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}
	
	if (!QuickSlots.IsValidIndex(QuickSlotIndex) || QuickSlots[QuickSlotIndex].IsEmpty())
	{
		return false;
	}
	
	FGameplayTag ItemTag = QuickSlots[QuickSlotIndex].ItemTypeTag;
	
	if (ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("ItemType.Consumable"))))
	{
		QuickSlots[QuickSlotIndex].Quantity -= 1;
        
		if (QuickSlots[QuickSlotIndex].Quantity <= 0)
		{
			QuickSlots[QuickSlotIndex].ItemTypeTag = FGameplayTag::EmptyTag;
			QuickSlots[QuickSlotIndex].Quantity = 0;
		}

		OnQuickSlotUpdated.Broadcast();

		OnItemUsed.Broadcast(ItemTag);

		FString DebugMsg = FString::Printf(TEXT("[소모품 사용] %s (남은 수량: %d)"), *ItemTag.ToString(), QuickSlots[QuickSlotIndex].Quantity);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, DebugMsg);

		return true;
	}
	
	else if (ItemTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("ItemType.Equipment"))))
	{
		OnItemUsed.Broadcast(ItemTag);

		FString DebugMsg = FString::Printf(TEXT("[무기 장착] %s"), *ItemTag.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, DebugMsg);

		return true;
	}
	
	return false;
}

bool UNCPlayerInventoryComponent::DropItem(int32 SlotIndex, int32 Quantity)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}
	
	if (!Items.IsValidIndex(SlotIndex) || Items[SlotIndex].IsEmpty() || Quantity <= 0)
	{
		return false;
	}
	
	FGameplayTag ItemTag = Items[SlotIndex].ItemTypeTag;
	
	int32 DropQuantity = FMath::Min(Quantity, Items[SlotIndex].Quantity);
	
	AActor* OwnerActor = GetOwner();
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
			SpawnedItemActor->InitializeItemData(ItemTag, DropQuantity);
		}
	}
	
	// 임시 디버그
	FString DebugMsg = FString::Printf(TEXT("%s 아이템 %d개 드롭"), *ItemTag.ToString(), DropQuantity);
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, DebugMsg);
	
	return RemoveItem(SlotIndex, DropQuantity);
}

bool UNCPlayerInventoryComponent::LootItem(class ANCItemActor* ItemToLoot)
{
	if (!GetOwner()->HasAuthority() || !ItemToLoot)
	{
		return false;
	}
	
	FGameplayTag LootTag = ItemToLoot->ItemTypeTag; 
	int32 LootQuantity = ItemToLoot->Quantity;
	
	bool bAdded = AddItem(LootTag, LootQuantity);
	
	if (bAdded)
	{
		ItemToLoot->Destroy();
		return true;
	}
	
	return false;
}