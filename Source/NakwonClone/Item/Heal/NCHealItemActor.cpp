#include "NCHealItemActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Common/NCGameplayTags.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"

ANCHealItemActor::ANCHealItemActor()
{
	HealAmount = 30.f;
	ItemTypeTag = NCItemTag::Heal;
}

void ANCHealItemActor::UseItem(ACharacter* User)
{
	Super::UseItem(User);

	if (!HasAuthority() || !User) return;

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(User);
	if (!ASCInterface) return;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC) return;

	const float CurrentHealth = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetHealthAttribute());
	const float CurrentMax = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxHealthAttribute());
	const float NewHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, CurrentMax);

	ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetHealthAttribute(), NewHealth);

	UE_LOG(LogTemp, Log, TEXT("[NCHealItem] %s 체력 회복: %.1f -> %.1f"),
		*User->GetName(), CurrentHealth, NewHealth);
}

void ANCHealItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANCHealItemActor, HealAmount);
}