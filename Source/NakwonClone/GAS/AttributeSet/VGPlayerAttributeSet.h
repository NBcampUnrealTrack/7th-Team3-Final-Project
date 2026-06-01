// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "VGPlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class NAKWONCLONE_API UVGPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UVGPlayerAttributeSet();
	
	// 체력
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, Health)

	// 최대 체력
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, MaxHealth)

	// 스테미나
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, Stamina)

	// 최대 스테미나
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, MaxStamina)

	// 감염도
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Infection;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, Infection)

	// 최대 감염도
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData MaxInfection;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, MaxInfection)

	// 크레딧
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	FGameplayAttributeData Credits;
	ATTRIBUTE_ACCESSORS(UVGPlayerAttributeSet, Credits)

	// 스탯이 변경되기 전 호출 (값 클램핑)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	// 스탯 변경 후
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
