// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"

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
	MonsterAttributeSet = CreateDefaultSubobject<UVGMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

UAbilitySystemComponent* AVGMonsterCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AVGMonsterCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] BeginPlay 호출됨: %s"), *GetName());

	AIController = Cast<AVGMonsterAIControllerBase>(GetController());

	if (!AIController)
	{
		UE_LOG(LogMonster, Error, TEXT("[MonsterBase] AIController 캐스팅 실패: %s"), *GetName());
	}
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
	if (MonsterAttributeSet)
	{
		MonsterAttributeSet->OnHitReceived.AddDynamic(this, &AVGMonsterCharacterBase::HandleHit);
		MonsterAttributeSet->OnDead.AddDynamic(this, &AVGMonsterCharacterBase::HandleDead);
	}
}

void AVGMonsterCharacterBase::HandleHit()
{
	if (MonsterAttributeSet->GetHealth() <= 0.f) return;
	
	// 뒤로 밀려남
	FVector PushBack = -GetActorForwardVector();
	LaunchCharacter(PushBack * 300.f, true, false);
	
	// Blackboard에 bIsHit Set
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(FName("BIsHit"), true);
		}
	}
}

void AVGMonsterCharacterBase::HandleDead()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}

	SetActorEnableCollision(false);

	float Duration = PlayAnimMontage(GetRandomMontage(AnimDead));

	FTimerHandle FreezeHandle;
	GetWorldTimerManager().SetTimer(FreezeHandle, [this]()
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Pause(GetRandomMontage(AnimDead));
		}
	}, 1.16f, false);

	SetLifeSpan(200.f);
}