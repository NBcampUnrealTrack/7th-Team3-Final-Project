#include "NCItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"

ANCItemActor::ANCItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;
	
	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemMesh->SetCollisionObjectType(ECC_WorldStatic);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(ItemMesh);
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	InteractionWidget->SetVisibility(false);
}

void ANCItemActor::BeginPlay()
{
	Super::BeginPlay();
}

void ANCItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCItemActor, ItemTypeTag);
	DOREPLIFETIME(ANCItemActor, Quantity);
	DOREPLIFETIME(ANCItemActor, ItemID);    
	DOREPLIFETIME(ANCItemActor, ItemMeshAsset);
}

// 자식 클래스에서 오버라이드
void ANCItemActor::UseItem(ACharacter* User)
{
	
}

void ANCItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(Interactor);
	if (!PlayerCharacter)
	{
		return;
	}

	UNCPlayerInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
	if (InventoryComp)
	{
		InventoryComp->LootItem(this);
	}
}

bool ANCItemActor::CanInteract_Implementation(AActor* Interactor)
{
	// 임시로 무조건 상호작용 가능하도록 true 반환
	return true;
}

FText ANCItemActor::GetInteractPrompt_Implementation()
{
	return FText::FromString(TEXT("아이템 줍기"));
}

void ANCItemActor::ToggleHighlight_Implementation(bool bHighlight)
{
	ItemMesh->SetRenderCustomDepth(bHighlight);

	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(bHighlight);
	}
	
	if (ItemMesh) 
	{
		ItemMesh->SetRenderCustomDepth(bHighlight);
		ItemMesh->SetCustomDepthStencilValue(1);
	}
}

void ANCItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!ItemID.IsNone())
	{
		UDataTable* LoadedItemDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/NakwonClone/Blueprints/Item/ItemData/DT_ItemTypeData.DT_ItemTypeData"));
		
		if (LoadedItemDataTable)
		{
			FItemData* FoundData = LoadedItemDataTable->FindRow<FItemData>(ItemID, TEXT("ActorConstruction"));
			
			if (FoundData)
			{
				ItemTypeTag = FoundData->ItemTypeTag;
				
				if (ItemMesh && FoundData->ItemMesh)
				{
					ItemMesh->SetStaticMesh(FoundData->ItemMesh);
				}
			}
		}
	}
}

void ANCItemActor::OnRep_ItemMeshAsset()
{
	if (ItemMesh && ItemMeshAsset)
	{
		ItemMesh->SetStaticMesh(ItemMeshAsset);
	}
}

void ANCItemActor::InitializeItemData(FName InItemID, FGameplayTag InTag, int32 InQuantity, UStaticMesh* InMesh)
{
	if (HasAuthority())
	{
		ItemID = InItemID;
		ItemTypeTag = InTag;
		Quantity = InQuantity;
		
		ItemMeshAsset = InMesh;
		OnRep_ItemMeshAsset();
	}
}