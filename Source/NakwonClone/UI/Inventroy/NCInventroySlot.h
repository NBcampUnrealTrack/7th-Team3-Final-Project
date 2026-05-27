#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NCInventroySlot.generated.h"

UCLASS()
class NAKWONCLONE_API UNCInventroySlot : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION()
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void UpdateItemSlot(UTexture2D* ItemIcon);
};
