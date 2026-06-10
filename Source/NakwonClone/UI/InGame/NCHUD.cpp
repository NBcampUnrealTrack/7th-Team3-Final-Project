#include "NCHud.h"
#include "AbilitySystemComponent.h"
#include "NCStaminaBar.h"
#include "Player/PlayerCharacter/NCBaseCharacter.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "UI/InGame/NCHPBar.h"

void UNCHud::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	ANCPlayerCharacter* Character =Cast<ANCPlayerCharacter>(PC->GetPawn());
	if (!Character)
	{
		return;
	}

	ASC = Character->GetAbilitySystemComponent();
	AttributeSet = Character->GetPlayerAttributeSet();
	if (!ASC || !AttributeSet)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UNCHud::OnHPChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute()).AddUObject(this, &UNCHud::OnStaminaChanged);
	
	if (HPBarWidget)
	{
		HPBarWidget->UpdateHP(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	}
	if (StaminaBarWidget)
	{
		StaminaBarWidget->UpdateStaminaBar(AttributeSet->GetStamina(), AttributeSet->GetMaxStamina());
	}
}

void UNCHud::OnHPChanged(const FOnAttributeChangeData& Data)
{
	if (!HPBarWidget || !AttributeSet)
	{
		return;
	}

	HPBarWidget->UpdateHP(Data.NewValue, AttributeSet->GetMaxHealth());
}

void UNCHud::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (!StaminaBarWidget || !AttributeSet)
	{
		return;
	}

	StaminaBarWidget->UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
}

