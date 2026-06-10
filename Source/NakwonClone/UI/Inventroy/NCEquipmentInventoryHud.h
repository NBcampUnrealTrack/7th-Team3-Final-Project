#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NCEquipmentInventorySlot.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"
#include "Components/UniformGridPanel.h"
#include "NCEquipmentInventoryHud.generated.h"

UCLASS()
class NAKWONCLONE_API UNCEquipmentInventoryHud : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void InitWithInventory(UNCInventoryBaseComponent* InInventory);

	UFUNCTION(BlueprintCallable)
	void UpdateEquipmentItemSlot();
	
	UPROPERTY()
	TObjectPtr<UNCInventoryBaseComponent> InventoryComp;
	
	UPROPERTY()
	TArray<TObjectPtr<UNCEquipmentInventorySlot>> SlotWidgets;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UNCEquipmentInventorySlot> SlotClass;
};
