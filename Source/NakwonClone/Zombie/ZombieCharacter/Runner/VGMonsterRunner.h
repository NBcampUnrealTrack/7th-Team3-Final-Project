// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "VGMonsterRunner.generated.h"

UCLASS()
class NAKWONCLONE_API AVGMonsterRunner : public AVGMonsterCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVGMonsterRunner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
