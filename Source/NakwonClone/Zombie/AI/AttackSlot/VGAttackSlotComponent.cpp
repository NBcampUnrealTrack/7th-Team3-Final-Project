// Fill out your copyright notice in the Description page of Project Settings.


#include "VGAttackSlotComponent.h"
#include "NavigationSystem.h"


UVGAttackSlotComponent::UVGAttackSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SlotOwners.SetNum(MaxSlots);
}

bool UVGAttackSlotComponent::RequestSlot(AActor* Requester, int32& OutSlotIndex)
{
	if (!Requester) return false;
	if (SlotOwners.Num() != MaxSlots) SlotOwners.SetNum(MaxSlots);

	for (int32 i = 0; i < SlotOwners.Num(); ++i)
		if (SlotOwners[i].Get() == Requester) { OutSlotIndex = i; return true; }

	int32 BestIndex = -1;
	float BestDistSq = TNumericLimits<float>::Max();
	const FVector RequesterLoc = Requester->GetActorLocation();
	for (int32 i = 0; i < SlotOwners.Num(); ++i)
	{
		if (SlotOwners[i].IsValid()) continue;
		const float DistSq = FVector::DistSquared(RequesterLoc, GetSlotLocation(i));
		if (DistSq < BestDistSq) { BestDistSq = DistSq; BestIndex = i; }
	}
	if (BestIndex == -1) return false;

	SlotOwners[BestIndex] = Requester;
	OutSlotIndex = BestIndex;
	return true;
}

void UVGAttackSlotComponent::ReleaseSlot(AActor* Requester)
{
	if (!Requester) return;
	for (auto& Owner : SlotOwners)
	{
		if (Owner.Get() == Requester)
		{
			Owner = nullptr;
			break;
		}
	}
}

FVector UVGAttackSlotComponent::GetSlotLocation(int32 SlotIndex) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !SlotOwners.IsValidIndex(SlotIndex)) return FVector::ZeroVector;

	const float AngleStep = 360.f / FMath::Max(1, MaxSlots);
	const float AngleDeg = AngleStep * SlotIndex;
	const FVector Offset = FRotator(0.f, AngleDeg, 0.f).RotateVector(FVector(SlotRadius, 0.f, 0.f));
	const FVector RawLocation = Owner->GetActorLocation() + Offset;

	if (const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(RawLocation, NavLoc, FVector(100.f, 100.f, 200.f)))
		{
			return NavLoc.Location;
		}
	}

	return RawLocation;
}

bool UVGAttackSlotComponent::RequestWaitSlot(AActor* Requester, int32& OutSlotIndex)
{
	if (!Requester) return false;
	if (WaitSlotOwners.Num() != MaxWaitSlots) WaitSlotOwners.SetNum(MaxWaitSlots);

	for (int32 i = 0; i < WaitSlotOwners.Num(); ++i)
		if (WaitSlotOwners[i].Get() == Requester) { OutSlotIndex = i; return true; }

	for (int32 i = 0; i < WaitSlotOwners.Num(); ++i)
		if (!WaitSlotOwners[i].IsValid()) { WaitSlotOwners[i] = Requester; OutSlotIndex = i; return true; }

	return false;
}

void UVGAttackSlotComponent::ReleaseWaitSlot(AActor* Requester)
{
	if (!Requester) return;
	for (auto& Owner : WaitSlotOwners)
		if (Owner.Get() == Requester) { Owner = nullptr; break; }
}

FVector UVGAttackSlotComponent::GetWaitSlotLocation(int32 SlotIndex) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !WaitSlotOwners.IsValidIndex(SlotIndex)) return FVector::ZeroVector;

	const float AngleStep = 360.f / FMath::Max(1, MaxWaitSlots);
	const float AngleDeg = AngleStep * SlotIndex;
	const FVector Offset = FRotator(0.f, AngleDeg, 0.f).RotateVector(FVector(WaitSlotRadius, 0.f, 0.f));
	const FVector RawLocation = Owner->GetActorLocation() + Offset;

	if (const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(RawLocation, NavLoc, FVector(100.f, 100.f, 200.f)))
		{
			return NavLoc.Location;
		}
	}

	return RawLocation;
}