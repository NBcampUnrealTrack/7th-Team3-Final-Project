// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Item/Shop/NCShopItemActor.h"
#include "NCShopGunItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCShopGunItemActor : public ANCShopItemActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Gun")
	FName GunID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Gun")
	float OldGunDropLifeSpan = 60.f;

protected:
	virtual void OnPurchased(ANCPlayerCharacter* Player) override;
};
