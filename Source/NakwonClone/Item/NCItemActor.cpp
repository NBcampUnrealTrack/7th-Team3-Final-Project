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
	
	ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	ItemMesh->SetSimulatePhysics(true);
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

void ANCItemActor::InitializeItemData(FGameplayTag InTag, int32 InQuantity)
{
	if (HasAuthority())
	{
		ItemTypeTag = InTag;
		Quantity = InQuantity;
	}
}


