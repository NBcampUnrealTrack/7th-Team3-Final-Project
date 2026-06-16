#include "ANCLootBoxActor.h"
#include "Components/StaticMeshComponent.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"
#include "NakwonClone/Item/Data/NCLootDropData.h"
#include "NakwonClone/Player/PlayerController/NCPlayerController.h"
#include "Common/NCGameplayTags.h"
#include "Net/UnrealNetwork.h"

AANCLootBoxActor::AANCLootBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	RootComponent = BoxMesh;

	LootInventory = CreateDefaultSubobject<UNCInventoryBaseComponent>(TEXT("LootInventory"));
	LootInventory->SetIsReplicated(true);
}

void AANCLootBoxActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GenerateLoot();
	}
}

void AANCLootBoxActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AANCLootBoxActor, StateTags);
}


void AANCLootBoxActor::GenerateLoot()
{
	if (!LootInventory || !LootInventory->ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LootBox] GenerateLoot: LootInventory에 ItemDataTable이 설정되지 않음"));
		return;
	}
	
	for (const TPair<FName, int32>& FixedItem : FixedLootItems)
	{
		FItemData* ItemData = LootInventory->ItemDataTable->FindRow<FItemData>(FixedItem.Key, TEXT("LootBox_FixedItem"));
		if (ItemData)
		{
			LootInventory->AddItem(FixedItem.Key, ItemData->ItemTypeTag, FixedItem.Value);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootBox] FixedItem '%s'을 ItemDataTable에서 찾을 수 없음"), *FixedItem.Key.ToString());
		}
	}

	// 랜덤 확률 아이템 스폰
	if (RandomDropTable)
	{
		TArray<FNCLootDropData*> AllDropData;
		RandomDropTable->GetAllRows<FNCLootDropData>(TEXT("LootBox_RandomDrop"), AllDropData);

		if (AllDropData.Num() > 0)
		{
			float TotalWeight = 0.0f;
			for (const FNCLootDropData* DropData : AllDropData)
			{
				TotalWeight += DropData->DropWeight;
			}

			for (int32 i = 0; i < RandomRollCount; ++i)
			{
				float RandomRoll = FMath::FRandRange(0.0f, TotalWeight);
				float CurrentWeight = 0.0f;

				for (const FNCLootDropData* DropData : AllDropData)
				{
					CurrentWeight += DropData->DropWeight;
                    
					if (RandomRoll <= CurrentWeight)
					{
						FItemData* ItemData = LootInventory->ItemDataTable->FindRow<FItemData>(DropData->ItemID, TEXT("LootBox_RandomItem"));
						if (ItemData)
						{
							int32 DropQuantity = FMath::RandRange(DropData->MinQuantity, DropData->MaxQuantity);
							LootInventory->AddItem(DropData->ItemID, ItemData->ItemTypeTag, DropQuantity);
						}
						break; 
						
					}
				}
			}
		}
	}
}

void AANCLootBoxActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return;
	}
	
	if (HasAuthority())
	{
		if (StateTags.HasTagExact(NCLootBox::State_BeingLooted))
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootBox] 이미 다른 플레이어가 파밍 중입니다"));
			return;
		}

		StateTags.AddTag(NCLootBox::State_BeingLooted);
		CurrentLooter = Interactor;

		UE_LOG(LogTemp, Log, TEXT("[LootBox] 서버: %s 가 상자를 열었습니다."), *Interactor->GetName());

		if (APawn* InteractorPawn = Cast<APawn>(Interactor))
		{
			if (ANCPlayerController* NCPC = Cast<ANCPlayerController>(InteractorPawn->GetController()))
			{
				NCPC->Client_OpenLootBoxUI(this);
			}
		}
	}
}

bool AANCLootBoxActor::CanInteract_Implementation(AActor* Interactor)
{
	return !StateTags.HasTagExact(NCLootBox::State_BeingLooted);
}

void AANCLootBoxActor::ToggleHighlight_Implementation(bool bHighlight)
{
	if (BoxMesh)
	{
		BoxMesh->SetRenderCustomDepth(bHighlight);
	}
}

void AANCLootBoxActor::EndLooting()
{
	if (HasAuthority())
	{
		StateTags.RemoveTag(NCLootBox::State_BeingLooted);
		CurrentLooter = nullptr;
		UE_LOG(LogTemp, Log, TEXT("[LootBox] 상자 파밍 종료, 잠금 해제"));
	}
}
