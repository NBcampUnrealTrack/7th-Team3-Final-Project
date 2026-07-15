#pragma once

#include "CoreMinimal.h"
#include "NCInventroySlot.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NCInventoryHud.generated.h"

UCLASS()
class NAKWONCLONE_API UNCInventoryHud : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	void AmmoUpdate(ANCPlayerCharacter* Player);
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoText;

	//virtual void NativeConstruct() override;
	
	// UFUNCTION(BlueprintCallable)
	// void InitWithInventory(UNCInventoryBaseComponent* InInventory);
	//
	// UFUNCTION()
	// void UpdateItemSlot();
	//
	// void CreateSlots();
	//
	// UPROPERTY(meta = (BindWidget))
	// UUniformGridPanel* InventoryGrid;
	//
	// UPROPERTY()
	// TObjectPtr<UNCInventoryBaseComponent> InventoryComp;
	//
	// UPROPERTY()
	// TArray<TObjectPtr<UNCInventroySlot>> SlotWidgets; 
	//
	// UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	// TSubclassOf<UNCInventroySlot> SlotClass;
};
