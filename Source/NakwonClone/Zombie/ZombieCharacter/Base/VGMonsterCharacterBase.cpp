// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"

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
		MonsterAttributeSet->OnDead.AddDynamic(this, &AVGMonsterCharacterBase::HandleDead);
		MonsterAttributeSet->OnHitReceived.AddDynamic(this, &AVGMonsterCharacterBase::HandleHit);
	}
}

// HandleDead()
void AVGMonsterCharacterBase::HandleDead()
{
	SetActorEnableCollision(false);
	
	float Duration = PlayAnimMontage(GetRandomMontage(AnimDead));
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
 	{
		AIC->StopMovement();
		AIC->UnPossess();
	}
	
	// 사망 애니메이션이 끝나면 래그돌 전환
	GetWorldTimerManager().SetTimer(DeadTimerHandle, [this]()
	{
		OnStartRagdoll();
	}, Duration, false);
	
	SetLifeSpan(200.f);
}

void AVGMonsterCharacterBase::OnStartRagdoll()
{
	USkeletalMeshComponent* SkelMesh  = GetMesh();
	if (!SkelMesh )
	{
		return;
	}
	
	SkelMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	
	SkelMesh->SetAllBodiesSimulatePhysics(true);
	SkelMesh->SetPhysicsBlendWeight(1.f);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVGMonsterCharacterBase::HandleHit()
{
	if (MonsterAttributeSet->GetHealth() <= 0.f) return;
	
	bIsHit = true;
	PlayAnimMontage(GetRandomMontage(AnimHit));
	
	GetWorldTimerManager().SetTimer(HitTimerHandle, [this]()
	{
		bIsHit = false;
		
		if (AIController)
		{
			if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
			{
				Blackboard->SetValueAsBool(FName("BIsHit"), false);
			}
		}
	}, 0.5f, false);
	
	// 뒤로 밀려남
	FVector PushBack = -GetActorForwardVector();
	LaunchCharacter(PushBack * 300.f, true, false);
}

UAnimMontage* AVGMonsterCharacterBase::GetRandomMontage(const TArray<TObjectPtr<UAnimMontage>>& Montages)
{
	if (Montages.IsEmpty())
	{
		return nullptr;
	}
	return Montages[FMath::RandRange(0, Montages.Num() - 1)];
}