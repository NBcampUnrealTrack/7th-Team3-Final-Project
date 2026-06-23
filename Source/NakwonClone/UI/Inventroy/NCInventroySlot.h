#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "NCInventroySlot.generated.h"

class AANCLootBoxActor;
class UNCInventoryBaseComponent;

UCLASS()
class NAKWONCLONE_API UNCInventroySlot : public UUserWidget
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
	UImage* ItemImage;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCountText;
	
	UPROPERTY(BlueprintReadWrite)
	int32 SlotIndex;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ItemTag;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Quantity;
	
	// 하상빈 추가
	UPROPERTY(BlueprintReadWrite)
	bool bIsLootBoxSlot = false;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AANCLootBoxActor> LootBoxRef;

	// 이 슬롯이 속한 인벤토리 (가방=PlayerInventory, 창고=StashInventory)
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UNCInventoryBaseComponent> OwningInventory;
};
