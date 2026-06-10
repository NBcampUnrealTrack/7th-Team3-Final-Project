#pragma once

#include "CoreMinimal.h"
#include "NCHPBar.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "NCHud.generated.h"

class UNHPBar;
class UNCStaminaBar;
class UAbilitySystemComponent;
class UVGPlayerAttributeSet;

UCLASS()
class NAKWONCLONE_API UNCHud : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	void OnHPChanged(const FOnAttributeChangeData& Data);
	void OnStaminaChanged(const FOnAttributeChangeData& Data);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNCHPBar> HPBarWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNCStaminaBar> StaminaBarWidget;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UVGPlayerAttributeSet> AttributeSet;
};
