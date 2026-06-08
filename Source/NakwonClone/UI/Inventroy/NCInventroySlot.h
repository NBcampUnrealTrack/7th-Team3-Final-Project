#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "NCInventroySlot.generated.h"

UCLASS()
class NAKWONCLONE_API UNCInventroySlot : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION()
	virtual void NativeConstruct() override;
	
public:
	UFUNCTION(BlueprintCallable)
	void SetSlotData(int32 InIndex, FGameplayTag InTag, int32 InQuantity, FName InItemID);
	
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
};
