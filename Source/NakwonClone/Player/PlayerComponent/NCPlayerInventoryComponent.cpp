#include "NCPlayerInventoryComponent.h"

#include "Framwork/PlayerState/NCPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Item/NCItemActor.h"
#include "Player/PlayerAnimation/NCCombatComponent.h"
#include "Player/PlayerCharacter/NCBaseCharacter.h"
#include "Common/NCSaveGame.h"
#include "Item/ANCLootBoxActor.h"	
#include "Item/NCItemActor.h"

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

FInventorySlot UNCPlayerInventoryComponent::GetQuickSlotData(int32 SlotIndex) const
{
	if (QuickSlots.IsValidIndex(SlotIndex))
	{
		return QuickSlots[SlotIndex];
	}
    
	return FInventorySlot();
}

FInventorySlot UNCPlayerInventoryComponent::GetMainSlotData(int32 SlotIndex) const
{
	if (Items.IsValidIndex(SlotIndex))
	{
		return Items[SlotIndex];
	}
    
	return FInventorySlot();
}

void UNCPlayerInventoryComponent::ForceUnArm()
{
	if (CurrentEquippedSlotIndex == -1)
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
				CurrentEquippedSlotIndex = -1;
				
				FString DebugMsg = TEXT("[H키 : 맨손 전환]");
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, DebugMsg);	
			}
		}
	}
}

void UNCPlayerInventoryComponent::SaveInventoryData()
{
	UNCSaveGame* SaveInst = Cast<UNCSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("LobbyInventorySlot"), 0));
	if (!SaveInst)
	{
		SaveInst = Cast<UNCSaveGame>(UGameplayStatics::CreateSaveGameObject(UNCSaveGame::StaticClass()));
	}

	if (SaveInst)
	{
		SaveInst->PlayerInventoryItems = Items;
		SaveInst->PlayerQuickSlots = QuickSlots;

		UGameplayStatics::SaveGameToSlot(SaveInst, TEXT("LobbyInventorySlot"), 0);

		FString DebugMsg = TEXT("[Save] 인벤토리 및 퀵슬롯 저장 완료");
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, DebugMsg);
	}
}

void UNCPlayerInventoryComponent::LoadInventoryData()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("LobbyInventorySlot"), 0))
	{
		UNCSaveGame* LoadInst = Cast<UNCSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("LobbyInventorySlot"), 0));
		if (LoadInst)
		{
			Items = LoadInst->PlayerInventoryItems;
			QuickSlots = LoadInst->PlayerQuickSlots;

			if (QuickSlots.Num() != 4)
			{
				QuickSlots.Init(FInventorySlot(), 4);
			}

			OnInventoryUpdated.Broadcast();
			OnQuickSlotUpdated.Broadcast();

			FString DebugMsg = TEXT("[Load] 인벤토리 및 퀵슬롯 불러오기 완료");
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, DebugMsg);
		}
	}
}

void UNCPlayerInventoryComponent::TakeItemFromLootBox(AANCLootBoxActor* LootBox, int32 BoxSlotIndex,int32 PlayerSlotIndex)
{
	if (!LootBox)
	{
		return;
	}
	Server_TakeItemFromLootBox(LootBox, BoxSlotIndex, PlayerSlotIndex);
}

void UNCPlayerInventoryComponent::Server_TakeItemFromLootBox_Implementation(AANCLootBoxActor* LootBox,int32 BoxSlotIndex, int32 PlayerSlotIndex)
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
	
	if (!ItemTag.MatchesTag(NCItemType::Consumable)) 
	{
		return false;
	}
	
	FString DebugMsg = FString::Printf(TEXT("아이템 사용 태그: %s"), *ItemTag.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, DebugMsg);
	
	return RemoveItem(SlotIndex, 1);
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

	FGameplayTag ItemTag = Items[MainSlotIndex].ItemTypeTag;
	int32 TargetQuickSlotIndex = -1;

	if (ItemTag.MatchesTag(NCItemTag::Weapon))
	{
		if (QuickSlots[0].IsEmpty()) TargetQuickSlotIndex = 0;
		else if (QuickSlots[1].IsEmpty()) TargetQuickSlotIndex = 1;
		else TargetQuickSlotIndex = 0;
	}
	else if (ItemTag.MatchesTag(NCItemTag::Heal))
	{
		TargetQuickSlotIndex = 2;
	}
	else if (ItemTag.MatchesTag(NCItemTag::Food))
	{
		TargetQuickSlotIndex = 3;
	}

	if (TargetQuickSlotIndex != -1)
	{
		return EquipToQuickSlot(MainSlotIndex, TargetQuickSlotIndex);
	}

	return false;
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
	FName ItemID = Items[MainSlotIndex].ItemID;
	
	if (QuickSlotIndex == 0 || QuickSlotIndex == 1)
	{
		if (!ItemTag.MatchesTag(NCItemTag::Weapon)) return false;
	}
	
	else if (QuickSlotIndex == 2)
	{
		if (!ItemTag.MatchesTag(NCItemTag::Heal)) return false;
	}
	
	else if (QuickSlotIndex == 3)
	{
		if (!ItemTag.MatchesTag(NCItemTag::Food)) return false;
	}
	
	if (!QuickSlots[QuickSlotIndex].IsEmpty())
	{
		if (QuickSlots[QuickSlotIndex].ItemID == ItemID && QuickSlots[QuickSlotIndex].ItemTypeTag == ItemTag)
		{
			FItemData ItemData;
			if (GetItemDataByTag(ItemID, ItemTag, ItemData))
			{
				int32 RoomInQuickSlot = ItemData.MaxStackSize - QuickSlots[QuickSlotIndex].Quantity;
                
				if (RoomInQuickSlot > 0)
				{
					if (Items[MainSlotIndex].Quantity <= RoomInQuickSlot)
					{
						QuickSlots[QuickSlotIndex].Quantity += Items[MainSlotIndex].Quantity;
						
						Items[MainSlotIndex].ItemID = NAME_None;
						Items[MainSlotIndex].ItemTypeTag = FGameplayTag::EmptyTag;
						Items[MainSlotIndex].Quantity = 0;
					}
					else
					{
						QuickSlots[QuickSlotIndex].Quantity = ItemData.MaxStackSize;
						Items[MainSlotIndex].Quantity -= RoomInQuickSlot;
					}
                    
					OnInventoryUpdated.Broadcast();
					OnQuickSlotUpdated.Broadcast();
					return true;
				}
			}
		}
	}
	
	FInventorySlot TempSlot = QuickSlots[QuickSlotIndex];
	QuickSlots[QuickSlotIndex] = Items[MainSlotIndex];
	Items[MainSlotIndex] = TempSlot;
	
	if (QuickSlotIndex == 0 || QuickSlotIndex == 1)
	{
		if (ANCPlayerState* NCPS =  Cast<ANCPlayerState>(GetOwner()))
		{
			if (APawn* NCPawn =	NCPS->GetPawn())
			{
				if (UNCCombatComponent* NCCombatComponent = NCPawn->FindComponentByClass<UNCCombatComponent>())
				{
					NCCombatComponent->EquipWeapon(QuickSlots[QuickSlotIndex].WeaponInstance);
				}
			}
		}
	}
	
	OnInventoryUpdated.Broadcast();
	OnQuickSlotUpdated.Broadcast();
	
	return true;
}

bool UNCPlayerInventoryComponent::UnequipFromQuickSlot(int32 QuickSlotIndex, int32 MainSlotIndex)
{
	if (!GetOwner()->HasAuthority())
    {
       return false;
    }

    if (!QuickSlots.IsValidIndex(QuickSlotIndex) || !Items.IsValidIndex(MainSlotIndex))
    {
       return false;
    }
    if (QuickSlots[QuickSlotIndex].IsEmpty())
    {
       return false;
    }

    if (!Items[MainSlotIndex].IsEmpty())
    {
       FGameplayTag IncomingTag = Items[MainSlotIndex].ItemTypeTag;
       if (QuickSlotIndex == 0 || QuickSlotIndex == 1)
       {
       	if (!IncomingTag.MatchesTag(NCItemTag::Weapon)) return false;
       }
       else if (QuickSlotIndex == 2)
       {
       	if (!IncomingTag.MatchesTag(NCItemTag::Heal)) return false;
       }
       else if (QuickSlotIndex == 3)
       {
       	if (!IncomingTag.MatchesTag(NCItemTag::Food)) return false;
       }
    }

    if (QuickSlotIndex == 0 || QuickSlotIndex == 1)
    {
       if (CurrentEquippedSlotIndex == QuickSlotIndex)
       {
          if (ANCPlayerState* NCPS = Cast<ANCPlayerState>(GetOwner()))
          {
             if (APawn* NCPawn = NCPS->GetPawn())
             {
                if (UNCCombatComponent* NCCombatComp = NCPawn->FindComponentByClass<UNCCombatComponent>())
                {
                   NCCombatComp->UnEquipWeapon();
                   CurrentEquippedSlotIndex = -1;
                   
                   FString DebugMsg = FString::Printf(TEXT("[장비 해제] %d번 슬롯 무기를 가방으로 이동 및 맨손 전환"), QuickSlotIndex);
                   GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, DebugMsg);
                }
             }
          }
       }
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
	FName ItemID = QuickSlots[QuickSlotIndex].ItemID;
	
	if (ItemTag.MatchesTag(NCItemType::Consumable))
	{
		QuickSlots[QuickSlotIndex].Quantity -= 1;
        
		if (QuickSlots[QuickSlotIndex].Quantity <= 0)
		{
			QuickSlots[QuickSlotIndex].ItemID = NAME_None;
			QuickSlots[QuickSlotIndex].ItemTypeTag = FGameplayTag::EmptyTag;
			QuickSlots[QuickSlotIndex].Quantity = 0;
		}

		OnQuickSlotUpdated.Broadcast();
		
		if (ItemTag.MatchesTag(NCItemTag::Heal) || ItemTag.MatchesTag(NCItemTag::Food))
		{
			if (ConsumableDataTable)
			{
				if (FConsumableItemData* Data = ConsumableDataTable->FindRow<FConsumableItemData>(ItemID, TEXT("UseQuickSlot")))
				{
					PendingConsumableData = *Data;
					bHasPendingConsumable = true;
				}
			}
		}
		
		OnItemUsed.Broadcast(ItemTag);

		FString DebugMsg = FString::Printf(TEXT("[소모품 사용] %s (남은 수량: %d)"), *ItemTag.ToString(), QuickSlots[QuickSlotIndex].Quantity);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, DebugMsg);

		return true;
	}
	
	else if (ItemTag.MatchesTag(NCItemTag::Weapon))
	{
		if (ANCPlayerState* NCPS = Cast<ANCPlayerState>(GetOwner()))
		{
			if (APawn* NCPawn =	NCPS->GetPawn())
			{
				if (UNCCombatComponent* NCCombatComp = NCPawn->FindComponentByClass<UNCCombatComponent>())
				{
					if (CurrentEquippedSlotIndex == QuickSlotIndex)
					{
						return true;
					}
					
					if (CurrentEquippedSlotIndex != -1)
					{
						NCCombatComp->UnEquipWeapon();
					}
					
					FNCWeaponInstance WeapontoEquip = QuickSlots[QuickSlotIndex].WeaponInstance;
					
					if (WeapontoEquip.WeaponID.IsNone())
					{
						WeapontoEquip.WeaponID = QuickSlots[QuickSlotIndex].ItemID;
						WeapontoEquip.UniqueID = FGuid::NewGuid();
						WeapontoEquip.CurrentDurability = 100.0f;
						WeapontoEquip.bIsBroken = false;
					}
					
					NCCombatComp->EquipWeapon(WeapontoEquip);
					CurrentEquippedSlotIndex = QuickSlotIndex;
					
					FString DebugMsg = FString::Printf(TEXT("[무기 장착] 슬롯: %d, 아이디: %s"), QuickSlotIndex, *WeapontoEquip.WeaponID.ToString());
					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, DebugMsg);
				}
			}
		}
		
		OnItemUsed.Broadcast(ItemTag);
		return true;
	}
		
	return false;
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
					MeshToSet = FoundData->ItemMesh; // 옷 찾기 성공!
				}
			}
			
			SpawnedItemActor->InitializeItemData(DropItemID, ItemTag, DropQuantity, MeshToSet);
		}
	}

	FString DebugMsg = FString::Printf(TEXT("[Server] %s 아이템 %d개 드롭"), *ItemTag.ToString(), DropQuantity);
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, DebugMsg);

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
		return;
	}
}