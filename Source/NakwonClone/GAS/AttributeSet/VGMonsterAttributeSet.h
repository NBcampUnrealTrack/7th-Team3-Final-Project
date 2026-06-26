// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGHitTypes.h"
#include "VGMonsterAttributeSet.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitReceived, const FVGHitData&, HitData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class NAKWONCLONE_API UVGMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UVGMonsterAttributeSet();
	
	// 체력
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UVGMonsterAttributeSet, Health)
	
	// 일반 공격력
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UVGMonsterAttributeSet, Damage)
	
	// 물기 감염도
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FGameplayAttributeData BiteInfection;
	ATTRIBUTE_ACCESSORS(UVGMonsterAttributeSet, BiteInfection)
	
	// 이동 속도
	UPROPERTY(BlueprintReadWrite, Category = "Stats")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UVGMonsterAttributeSet, MoveSpeed)
	
	// BlueprintAssignable = 블루 프린트에서도 이 델리게이트에 함수를 연결할 수 있게 함
	// 몬스터 피격
	UPROPERTY(BlueprintAssignable)
	FOnHitReceived OnHitReceived;
	
	// 몬스터 사망
	UPROPERTY(BlueprintAssignable)
	FOnDead OnDead;
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
private:
	bool bIsDead = false;
};
