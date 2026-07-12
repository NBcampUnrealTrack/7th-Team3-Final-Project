// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCInfiniteAmmoItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCInfiniteAmmoItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|InfiniteAmmo")
	float Duration = 5.f;

	virtual void Interact_Implementation(AActor* Interactor) override;
};