#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "NCInventoryHud.generated.h"

UCLASS()
class NAKWONCLONE_API UNCInventoryHud : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* InventoryGrid;
	
	// UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	// TSubclassOf<UNCInventroySlot> SlotClass;
};
