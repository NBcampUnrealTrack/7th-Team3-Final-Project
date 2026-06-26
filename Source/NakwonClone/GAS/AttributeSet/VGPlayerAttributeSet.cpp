// Fill out your copyright notice in the Description page of Project Settings.

#include "VGPlayerAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"

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

		ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwningActor());

		if (GetHealth() <= 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PlayerAS] 사망"));

			if (Player)
			{
				Player->OnDead();
			}
			else if (ANCBaseCharacter* Character = Cast<ANCBaseCharacter>(GetOwningActor()))
			{
				Character->OnDead();
			}

			return;
		}

		AActor* Attacker =
			Cast<AActor>(Data.EffectSpec.GetContext().GetSourceObject());

		const float Delta = Data.EvaluatedData.Magnitude;   // 음수면 데미지

		if (Player)
		{
			Player->HandleHitReact(Attacker);

			// 피 튀김 cue — 데미지일 때만(회복은 제외)
			if (Delta < 0.f)
			{
				if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
				{
					// 맞은 위치: 좀비 공격이 넘겨준 HitResult 사용
					// (본 이름 있으면 그 본 위치 → 없으면 임팩트 지점 → 둘 다 없으면 액터 위치)
					FVector BloodLocation = Player->GetActorLocation();
					if (const FHitResult* Hit = Data.EffectSpec.GetContext().GetHitResult())
					{
						if (Hit->BoneName != NAME_None && Player->GetMesh())
							BloodLocation = Player->GetMesh()->GetSocketLocation(Hit->BoneName);
						else
							BloodLocation = Hit->ImpactPoint;
					}

					FGameplayCueParameters CueParams;
					CueParams.Location = BloodLocation;

					// 공격자 → 맞은 위치 방향 = 피가 튈 방향
					if (Attacker)
					{
						CueParams.Normal =
							(BloodLocation - Attacker->GetActorLocation()).GetSafeNormal();
					}

					ASC->ExecuteGameplayCue(
						FGameplayTag::RequestGameplayTag("GameplayCue.Melee.Hit"),
						CueParams);
				}
			}
		}
	}

	if (Data.EvaluatedData.Attribute == GetCreditsAttribute())
	{
		UE_LOG(LogTemp, Log, TEXT("[Credits] 현재 크레딧: %.0f"), GetCredits());
	}
}
