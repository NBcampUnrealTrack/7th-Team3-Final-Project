// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"


AVGMonsterCharacterBase::AVGMonsterCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 자식 클래스는 반드시 AI 컨트롤러를 장착하도록 강제
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	// 컨트롤러 회전 영향 제거
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* AVGMonsterCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AVGMonsterCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[MonsterBase] BeginPlay 호출됨: %s"), *GetName());

	AIController = Cast<AVGMonsterAIControllerBase>(GetController());

	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[MonsterBase] AIController 캐스팅 실패: %s"), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MonsterBase] AIController 캐스팅 성공: %s"), *AIController->GetName());
	}
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AVGMonsterCharacterBase::TakeDamage_Monster(float DamageAmount)
{
	if (CurrentHealth <= 0.f) return;

	CurrentHealth = FMath::Max(0.f, CurrentHealth - DamageAmount);
	UE_LOG(LogTemp, Log, TEXT("[Monster] %s 피격: %.1f / %.1f"), *GetName(), CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		Dead();
	}
}

void AVGMonsterCharacterBase::Dead()
{
	UE_LOG(LogTemp, Warning, TEXT("[Monster] %s 사망"), *GetName());

	// AI 중단
	if (AIController)
	{
		AIController->StopMovement();
	}

	// 추후: 사망 애니메이션, 래그돌, 제거 타이머 등 추가
}
