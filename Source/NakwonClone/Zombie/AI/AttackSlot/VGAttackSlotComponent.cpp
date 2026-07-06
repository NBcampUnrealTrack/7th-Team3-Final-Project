// Fill out your copyright notice in the Description page of Project Settings.


#include "VGAttackSlotComponent.h"


UVGAttackSlotComponent::UVGAttackSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SlotOwners.SetNum(MaxSlots);
}

bool UVGAttackSlotComponent::RequestSlot(AActor* Requester, int32& OutSlotIndex)
{
	if (!Requester) return false;

	if (SlotOwners.Num() != MaxSlots)
	{
		SlotOwners.SetNum(MaxSlots);
	}

	// 이미 슬롯 보유중이면 그대로 반환
	for (int32 i = 0; i < SlotOwners.Num(); ++i)
	{
		if (SlotOwners[i].Get() == Requester)
		{
			OutSlotIndex = i;
			return true;
		}
	}

	// 빈 슬롯(또는 소멸된 참조) 탐색
	for (int32 i = 0; i < SlotOwners.Num(); ++i)
	{
		if (!SlotOwners[i].IsValid())
		{
			SlotOwners[i] = Requester;
			OutSlotIndex = i;
			return true;
		}
	}

	return false;   // 슬롯 다 찼음
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
	return Owner->GetActorLocation() + Offset;
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
	return Owner->GetActorLocation() + Offset;
}