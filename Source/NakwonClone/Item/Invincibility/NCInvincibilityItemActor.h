// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCInvincibilityItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCInvincibilityItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCInvincibilityItemActor();

	UPROPERTY(EditAnywhere, Category = "Item|Invincibility")
	float Duration = 5.f;

	virtual void Interact_Implementation(AActor* Interactor) override;
};
