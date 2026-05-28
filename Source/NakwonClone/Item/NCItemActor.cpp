#include "NCItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

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

void ANCItemActor::InitializeItemData(FGameplayTag InTag, int32 InQuantity)
{
	if (HasAuthority())
	{
		ItemTypeTag = InTag;
		Quantity = InQuantity;
	}
}


