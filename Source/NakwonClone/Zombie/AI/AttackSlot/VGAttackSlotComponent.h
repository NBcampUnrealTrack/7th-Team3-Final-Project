// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VGAttackSlotComponent.generated.h"


UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UVGAttackSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVGAttackSlotComponent();
	
	// 슬롯 예약 시도. 이미 예약 중이면 그 슬롯 재사용 빈 슬롯 있으면 새로 점유
	bool RequestSlot(AActor* Requester, int32& OutSlotIndex);
	
	// 슬롯 반납
	void ReleaseSlot(AActor* Requester);
	
	// 슬롯의 현재 월드 좌표 (Owner 위치 기준 매번 재계산 -> Owner가 움직이면 따라감)
	FVector GetSlotLocation(int32 SlotIndex) const;
	
	bool RequestWaitSlot(AActor* Requester, int32& OutSlotIndex);
	void ReleaseWaitSlot(AActor* Requester);
	FVector GetWaitSlotLocation(int32 SlotIndex) const;
	
protected:
	// 동시 근접 공격 허용 인원 (= 슬롯 갯수)
	UPROPERTY(EditAnywhere, Category = "AttackSlot")
	int32 MaxSlots = 5;
	
	// 슬롯 반경
	UPROPERTY(EditAnywhere, Category = "AttackSlot")
	float SlotRadius = 50.f;
	
	UPROPERTY(EditAnywhere, Category = "AttackSlot|Wait")
	int32 MaxWaitSlots = 16;
	UPROPERTY(EditAnywhere, Category = "AttackSlot|Wait")
	float WaitSlotRadius = 200.f;
	
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> SlotOwners;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> WaitSlotOwners;
};
