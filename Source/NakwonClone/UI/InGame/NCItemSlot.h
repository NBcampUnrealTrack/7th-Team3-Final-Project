#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Inventory/NCInventoryType.h"
#include "NCItemSlot.generated.h"

UCLASS()
class NAKWONCLONE_API UNCItemSlot : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* ItemDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Config")
	int32 TargetSlotIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* LeftSlotDefaultImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* RightSlotDefaultImage;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UImage* LeftSlotImage;
	
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UImage* RightSlotImage;
	
	UPROPERTY(meta = (BindWidget))
	UImage* FirstItemSlotImage;
	
	UPROPERTY(meta = (BindWidget))
	UImage* SecondItemSlotImage;
	
	UPROPERTY(meta = (BindWidget))
	USizeBox* LeftSlotSizeBox;
	
	UPROPERTY(meta = (BindWidget))
	USizeBox* RightSlotSizeBox;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FirstItemSlotNumber;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SecondItemSlotNumber;
	
	UFUNCTION()
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void UpdateSlotVisual();
	void UpdateImage(UImage* TargetImage, const FInventorySlot& SlotData);
};
