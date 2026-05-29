#pragma once

#include "CoreMinimal.h"
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
	UFUNCTION()
	void SetSlotData(int32 InIndex, FGameplayTag InTag, int32 InQuantity);
	
	UPROPERTY(meta = (BindWidget))
	UImage* ItemImage;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCountText;
	
	int32 SlotIndex;
};
