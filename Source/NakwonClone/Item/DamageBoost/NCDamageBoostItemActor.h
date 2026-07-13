// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCDamageBoostItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCDamageBoostItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Item|DamageBoost")
	float Multiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Item|DamageBoost")
	float Duration = 5.f;

	virtual void Interact_Implementation(AActor* Interactor) override;
};
