// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterAttributeSet.h"

UVGMonsterAttributeSet::UVGMonsterAttributeSet()
{
	InitHealth(300.f);
	InitDamage(8.f);
	InitBiteInfection(30.f);
}

void UVGMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}
