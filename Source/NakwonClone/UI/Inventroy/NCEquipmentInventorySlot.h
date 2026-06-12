#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "NCEquipmentInventorySlot.generated.h"

UCLASS()
class NAKWONCLONE_API UNCEquipmentInventorySlot : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION()
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void SetSlotData(int32 InIndex, int32 InQuantity, FGameplayTag InTag, FName InItemID);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* ItemDataTable;
	
	UPROPERTY(BlueprintReadOnly)
	FName ItemID;
	
	UPROPERTY(meta = (BindWidget))
	UImage* EquipmentItemImage;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EquipmentItemQuantityTextBlock;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ItemTag;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Quantity;
};
