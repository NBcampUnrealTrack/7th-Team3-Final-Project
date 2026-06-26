// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "VGMonsterRunnerController.generated.h"



UCLASS()
class NAKWONCLONE_API AVGMonsterRunnerController : public AVGMonsterAIControllerBase
{
	GENERATED_BODY()

public:
	AVGMonsterRunnerController();

protected:
	virtual void BeginPlay() override;
	
};
