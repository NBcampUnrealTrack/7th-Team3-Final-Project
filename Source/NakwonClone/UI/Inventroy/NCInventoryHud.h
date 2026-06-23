#pragma once

#include "CoreMinimal.h"
#include "NCInventroySlot.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "NCInventoryHud.generated.h"

UCLASS()
class NAKWONCLONE_API UNCInventoryHud : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void InitWithInventory(UNCInventoryBaseComponent* InInventory);

	UFUNCTION()
	void UpdateItemSlot();

	void CreateSlots();
	
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* InventoryGrid;
	
	UPROPERTY()
	TObjectPtr<UNCInventoryBaseComponent> InventoryComp;
	
	UPROPERTY()
	TArray<TObjectPtr<UNCInventroySlot>> SlotWidgets; 
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UNCInventroySlot> SlotClass;
};
