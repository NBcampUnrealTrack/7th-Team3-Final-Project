// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "VGMonsterWalkerController.generated.h"

UCLASS()
class NAKWONCLONE_API AVGMonsterWalkerController : public AVGMonsterAIControllerBase
{
	GENERATED_BODY()

public:
	//헌호수정 - 부모(Detour Crowd 생성자)에 FObjectInitializer 전달
	AVGMonsterWalkerController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
};
