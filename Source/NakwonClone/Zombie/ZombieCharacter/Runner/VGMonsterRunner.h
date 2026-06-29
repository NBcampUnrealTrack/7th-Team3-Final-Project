// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "VGMonsterRunner.generated.h"

class UGameplayEffect;

UCLASS()
class NAKWONCLONE_API AVGMonsterRunner : public AVGMonsterCharacterBase
{
	GENERATED_BODY()

public:
	AVGMonsterRunner();

protected:
	virtual void BeginPlay() override;

#pragma region GAS GE 슬롯
public:
	UPROPERTY(EditAnywhere, Category = "Runner|Attack")
	TSubclassOf<UGameplayEffect> BiteEffectClass;
	
	UPROPERTY(EditAnywhere, Category = "Runner|Speed")
	TSubclassOf<UGameplayEffect> RunSpeedEffectClass;
#pragma endregion
};
