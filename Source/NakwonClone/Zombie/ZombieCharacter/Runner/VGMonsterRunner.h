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

#pragma region 코어/라이프사이클
public:
	AVGMonsterRunner();

protected:
	virtual void BeginPlay() override;
#pragma endregion
};
