// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "GameFramework/CharacterMovementComponent.h"

AVGMonsterWalker::AVGMonsterWalker()
{
	// 큐브 메시 컴포넌트 생성
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetRootComponent());
	
	BodyMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	BodyMesh->CanCharacterStepUpOn = ECB_No;
	BodyMesh->SetCanEverAffectNavigation(false);

	// 기본 큐브 메시 할당 (UE 기본 제공 에셋)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMesh.Object);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
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
	SetMonsterState(EMonsterState::Patrol);
}

void AVGMonsterWalker::TakeDamage_Monster(float DamageAmount)
{
	// Walker 고유 피격 반응 (예: 피격 시 잠깐 멈추는 등 추후 확장)
	Super::TakeDamage_Monster(DamageAmount);
}

void AVGMonsterWalker::SetMonsterState(EMonsterState NewState)
{
	if (!BodyMesh) return;

	UMaterialInterface* TargetMaterial = nullptr;

	switch (NewState)
	{
	case EMonsterState::Patrol:
		TargetMaterial = PatrolMaterial;
		break;
	case EMonsterState::Stop:
		TargetMaterial = AlertMaterial;
		break;
	case EMonsterState::Chase:
		TargetMaterial = ChaseMaterial;
		break;
	case EMonsterState::Attack:
		TargetMaterial = AttackMaterial;
		break;
	case EMonsterState::Dead:
		TargetMaterial = DeadMaterial;
		break;
	}

	if (TargetMaterial)
	{
		BodyMesh->SetMaterial(0, TargetMaterial);
	}
}
