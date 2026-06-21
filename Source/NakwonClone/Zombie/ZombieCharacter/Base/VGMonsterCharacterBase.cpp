// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterCharacterBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/NCGameplayTags.h"
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
	
	DetectionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("DetectionCapsule"));
	DetectionCapsule->SetupAttachment(RootComponent);
	DetectionCapsule->SetCapsuleSize(40.f, 90.f);
	DetectionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
	
	if (AbilitySystemComponent && MonsterAttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UVGMonsterAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AVGMonsterCharacterBase::OnMoveSpeedChanged);
	}
	
	SelectedMoveMontage = GetRandomMoveMontage();
	SelectedChaseMontage = GetRandomChaseMontage();
	SelectedStopMontage = GetRandomStopMontage();
	SelectedDeadMontage = GetRandomDeadMontage();
	
	DetectionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AVGMonsterCharacterBase::OnDetectionOverlap);
}

// HandleDead()
void AVGMonsterCharacterBase::HandleDead()
{
	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleDead 호출됨: %s"), *GetName());
	
	if (AIController)
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsDeadKey, true);
		}
	}
}

void AVGMonsterCharacterBase::OnStartRagdoll()
{
	USkeletalMeshComponent* SkelMesh  = GetMesh();
	if (!SkelMesh) return;
	
	SkelMesh->SetAllBodiesSimulatePhysics(true);
	SkelMesh->SetPhysicsBlendWeight(1.f);
	
	SkelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkelMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVGMonsterCharacterBase::HandleHit()
{
	if (MonsterAttributeSet->GetHealth() <= 0.f) return;
	
	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleHit 호출됨: %s"), *GetName());
	
	if (AIController)
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsHitKey, true);
			UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] bIsHit Set: true"));
		}
	}
	
	/*// 뒤로 밀려남
	FVector PushBack = -GetActorForwardVector();
	LaunchCharacter(PushBack * 300.f, true, false);*/
}

UAnimMontage* AVGMonsterCharacterBase::GetRandomMontage(const TArray<TObjectPtr<UAnimMontage>>& Montages)
{
	if (Montages.IsEmpty()) return nullptr;
	
	return Montages[FMath::RandRange(0, Montages.Num() - 1)];
}
void AVGMonsterCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	}
}

void AVGMonsterCharacterBase::OnDetectionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!AIController) return;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(NCCharacter::Player)) return;

	if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
	{
		Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsAwakeKey, true);
	}
}
