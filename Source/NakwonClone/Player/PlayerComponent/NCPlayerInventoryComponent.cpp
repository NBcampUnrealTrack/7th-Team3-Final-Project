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

