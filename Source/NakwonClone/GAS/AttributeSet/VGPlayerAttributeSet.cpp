// Fill out your copyright notice in the Description page of Project Settings.


#include "VGPlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

UVGPlayerAttributeSet::UVGPlayerAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitInfection(0.f);
	InitMaxInfection(100.f);
	InitCredits(0.f);
}

void UVGPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// 값이 범위를 벗어나지 않도록 클램핑
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
	if (Attribute == GetInfectionAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxInfection());
	}
	if (Attribute == GetCreditsAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UVGPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerAS] 체력 변경 : %.1f"), GetHealth());
		
		if (GetHealth() <= 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PlayerAS] 사망"));
		}
	}
	if (Data.EvaluatedData.Attribute == GetCreditsAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("[Credits] 현재 크레딧: %.0f"), GetCredits());
	}
}
