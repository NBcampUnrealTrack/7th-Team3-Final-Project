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

