// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCInstantHealItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCInstantHealItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCInstantHealItemActor();

	UPROPERTY(EditAnywhere, Category = "Item|Heal")
	float HealAmount = 30.f;

	virtual void Interact_Implementation(AActor* Interactor) override;
};
