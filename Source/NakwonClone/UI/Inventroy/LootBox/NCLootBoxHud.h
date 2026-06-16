#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NCLootBoxHud.generated.h"

class UNCInventoryBaseComponent;
class AANCLootBoxActor;
class UUniformGridPanel;
class UNCInventroySlot;

UCLASS()
class NAKWONCLONE_API UNCLootBoxHud : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitWithLootBox(AANCLootBoxActor* TargetBox, UNCInventoryBaseComponent* PlayerInventory);

	UFUNCTION(BlueprintCallable)
	void CloseLootBoxUI();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateLootSlots();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> LootGrid;

	UPROPERTY(EditDefaultsOnly, Category = "LootBox|UI")
	TSubclassOf<UNCInventroySlot> SlotClass;

	UPROPERTY()
	TArray<TObjectPtr<UNCInventroySlot>> LootSlotWidgets;

	UPROPERTY()
	TObjectPtr<UNCInventoryBaseComponent> LootInventoryComp;
	
	UPROPERTY()
	TObjectPtr<AANCLootBoxActor> CurrentLootBox;
};
