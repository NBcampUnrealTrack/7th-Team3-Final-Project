#include "ANCLootBoxActor.h"
#include "Components/StaticMeshComponent.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"
#include "NakwonClone/Item/Data/NCLootDropData.h"
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
	for (const TPair<FName, int32>& FixedItem : FixedLootItems)
	{
		// TODO: 가방에 아이템 넣기
		// 예: LootInventory->AddItem(FixedItem.Key, FixedItem.Value);
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
						int32 DropQuantity = FMath::RandRange(DropData->MinQuantity, DropData->MaxQuantity);
                        
						// TODO: 가방에 아이템 넣기
						// 예: LootInventory->AddItem(DropData->ItemID, DropQuantity);
                        
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
	
	if (!HasAuthority())
	{
		if (StateTags.HasTagExact(NCLootBox::State_BeingLooted))
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootBox] 이미 다른 플레이어가 파밍 중입니다"));
			return;
		}

		StateTags.AddTag(NCLootBox::State_BeingLooted);
		CurrentLooter = Interactor;

		UE_LOG(LogTemp, Log, TEXT("[LootBox] 서버: %s 가 상자를 열었습니다."), *Interactor->GetName());

		// TODO: Interactor의 Controller에 Client RPC 호출
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
