// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"

AVGMonsterWalker::AVGMonsterWalker()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
	
	MonsterAttributeSet = CreateDefaultSubobject<UVGMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

void AVGMonsterWalker::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도를 BeginPlay에서도 적용 (에디터 값 반영)
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	
	// 시작 시 순찰 상태로 초기화
	SetMonsterState(EMonsterState::Move);
}

void AVGMonsterWalker::SetMonsterState(EMonsterState NewState)
{
	UAnimMontage* TargetMontage = nullptr;

	switch (NewState)
	{
	case EMonsterState::Move:
		TargetMontage = AnimMove;
		break;
	case EMonsterState::Stop:
		TargetMontage = AnimStop;
		break;
	case EMonsterState::Chase:
		TargetMontage = AnimChase;
		break;
	case EMonsterState::Hit:
		TargetMontage = AnimHit;
		break;
	case EMonsterState::Attack:
		TargetMontage = AnimAttack;
		break;
	case EMonsterState::Dead:
		TargetMontage = AnimDead;
		break;
	}
	
	if (TargetMontage)
	{
		PlayAnimMontage(TargetMontage);
	}
}
