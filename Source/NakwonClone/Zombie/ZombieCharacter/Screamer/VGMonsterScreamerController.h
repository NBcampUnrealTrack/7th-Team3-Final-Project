// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "VGMonsterScreamerController.generated.h"

UCLASS()
class NAKWONCLONE_API AVGMonsterScreamerController : public AVGMonsterAIControllerBase
{
	GENERATED_BODY()

public:
	AVGMonsterScreamerController();

protected:
	virtual void BeginPlay() override;
};