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
	StaminaAmount = 0.f;
	InfectionReduceAmount = 0.f;

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

	// 체력 회복
	if (HealAmount > 0.f)
	{
		const float Current = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetHealthAttribute());
		const float Max = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxHealthAttribute());
		ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetHealthAttribute(),
			FMath::Clamp(Current + HealAmount, 0.f, Max));
	}

	// 스태미나 회복
	if (StaminaAmount > 0.f)
	{
		const float Current = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetStaminaAttribute());
		const float Max = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxStaminaAttribute());
		ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetStaminaAttribute(),
			FMath::Clamp(Current + StaminaAmount, 0.f, Max));
	}

	// 감염도 감소
	if (InfectionReduceAmount > 0.f)
	{
		const float Current = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetInfectionAttribute());
		ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetInfectionAttribute(),
			FMath::Max(Current - InfectionReduceAmount, 0.f));
	}

	UE_LOG(LogTemp, Log, TEXT("[NCHealItem] %s 사용 - 체력: %.1f 스태미나: %.1f 감염도감소: %.1f"),
		*User->GetName(), HealAmount, StaminaAmount, InfectionReduceAmount);
}

void ANCHealItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANCHealItemActor, HealAmount);
	DOREPLIFETIME(ANCHealItemActor, StaminaAmount);
	DOREPLIFETIME(ANCHealItemActor, InfectionReduceAmount);
}