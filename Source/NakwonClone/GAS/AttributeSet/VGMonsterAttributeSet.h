// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "VGMonsterAttributeSet.generated.h"

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
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
};
