// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterRunner.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"


AVGMonsterRunner::AVGMonsterRunner()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
}

void AVGMonsterRunner::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetCharacterMovement() && MonsterAttributeSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = MonsterAttributeSet->GetMoveSpeed();
	}
	
	if (RandomMesh.Num() > 0)
	{
		int32 RandIndex = FMath::RandRange(0, RandomMesh.Num() - 1);
		GetMesh()->SetSkeletalMesh(RandomMesh[RandIndex]);
	}
}

