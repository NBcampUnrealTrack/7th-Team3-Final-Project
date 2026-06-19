// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterAttributeSet.h"
#include "GameplayEffectExtension.h"

UVGMonsterAttributeSet::UVGMonsterAttributeSet()
{
	InitHealth(25.f);
	InitDamage(8.f);
	InitBiteInfection(30.f);
	InitMoveSpeed(40.f);
}

// GE가 Attribute 값을 변경한 직후에 자동으로 호출되는 함수
void UVGMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Health 어트리뷰트가 변경됐을 때만 처리
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if (GetHealth() <= 0.f)
		{
			// 사망
			OnDead.Broadcast();
		}
		else
		{
			// 피격
			OnHitReceived.Broadcast(); 
		}
	}
}

void UVGMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}