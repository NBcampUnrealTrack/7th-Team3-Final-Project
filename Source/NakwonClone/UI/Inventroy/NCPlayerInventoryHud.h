#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NCInventroySlot.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"
#include "Components/UniformGridPanel.h"
#include "NCPlayerInventoryHud.generated.h"

UCLASS()
class NAKWONCLONE_API UNCPlayerInventoryHud : public UUserWidget
{
	GENERATED_BODY()
	
		
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void InitWithInventory(UNCInventoryBaseComponent* InInventory);

	UFUNCTION()
	void UpdateItemSlot();
	
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* PlayerInventoryGrid;
	
	UPROPERTY()
	TObjectPtr<UNCInventoryBaseComponent> InventoryComp;
	
	UPROPERTY()
	TArray<TObjectPtr<UNCInventroySlot>> SlotWidgets; 
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UNCInventroySlot> SlotClass;
};
