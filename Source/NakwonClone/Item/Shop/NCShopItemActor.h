// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCShopItemActor.generated.h"

class ANCPlayerCharacter;
class UTextRenderComponent;

UCLASS(Abstract)
class NAKWONCLONE_API ANCShopItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCShopItemActor();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	int32 RequiredScore = 0;
	
	UFUNCTION(BlueprintPure, Category = "Shop")
	int32 GetRequiredScore() const { return RequiredScore; }

	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UTextRenderComponent> ScoreTextComponent;
	
	// 구매 성공 시 실제 지급 로직 (자식 클래스에서 구현)
	virtual void OnPurchased(ANCPlayerCharacter* Player) PURE_VIRTUAL(ANCShopItemActor::OnPurchased, );
};
