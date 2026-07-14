// Fill out your copyright notice in the Description page of Project Settings.


#include "NCInstantHealItemActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerController/NCPlayerController.h"

ANCInstantHealItemActor::ANCInstantHealItemActor()
{
	HealAmount = 30.f;
}

void ANCInstantHealItemActor::Interact_Implementation(AActor* Interactor)
{
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Interactor);
	if (!ASCInterface) return;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC) return;

	const float Current = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetHealthAttribute());
	const float Max     = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetMaxHealthAttribute());
	ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetHealthAttribute(),
		FMath::Clamp(Current + HealAmount, 0.f, Max));

	if (ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor))
	{
		if (ANCPlayerController* NCPC = Cast<ANCPlayerController>(Player->GetController()))
		{
			NCPC->Client_ShowNotification(FText::FromString(TEXT("체력이 회복되었습니다")), FLinearColor::Green);
		}
	}

	ConsumeItem(Interactor);
}
