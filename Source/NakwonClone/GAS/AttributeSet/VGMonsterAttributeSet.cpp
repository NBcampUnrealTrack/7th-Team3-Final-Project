// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterAttributeSet.h"
#include "GameplayEffectExtension.h"

UVGMonsterAttributeSet::UVGMonsterAttributeSet()
{
	InitHealth(100.f);
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
		FVGHitData HitData;
		if (const FHitResult* Hit = Data.EffectSpec.GetContext().GetHitResult())
		{
			HitData.BodyPart = ClassifyBodyPart(Hit->BoneName);
			HitData.HitLocation = Hit->ImpactPoint;
			HitData.HitNormal = Hit->ImpactNormal;
		}

		const float Delta = Data.EvaluatedData.Magnitude;
		if (Delta < 0.f)
		{
			const float Mult = GetBodyPartDamageMultiplier(HitData.BodyPart);
			if (Mult != 1.f)
			{
				const float Extra = Delta * (Mult - 1.f);
				SetHealth(FMath::Max(0.f, GetHealth() + Extra));
			}
		}

		if (GetHealth() <= 0.f)
		{
			// 사망
			if (!bIsDead)
			{
				bIsDead = true;
				OnDead.Broadcast();
			}
		}
		else
		{
			// 피격
			if (!bIsDead)
			{
				OnHitReceived.Broadcast(HitData);
			}
		}
	}
}

// GE가 Attribute 값을 변경하기 직전에 호출되는 함수
void UVGMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Max(0.f, NewValue);
	}
}