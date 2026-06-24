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
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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
	
	SelectedStopMontage = GetRandomStopMontage();
	SelectedDeadMontage = GetRandomDeadMontage();
	
	DetectionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AVGMonsterCharacterBase::OnDetectionOverlap);

	// H
	if (HasAuthority())
	{
		StartHowlTimer();
	}
	
	int32 MoveIndex = FMath::RandRange(0, AnimMove.Num()-1);
	SelectedMoveMontage = AnimMove[MoveIndex];
	SelectedMoveLevel = MoveIndex + 1;
	
	int32 ChaseIndex = FMath::RandRange(0, AnimChase.Num() - 1);
	SelectedChaseMontage = AnimChase[ChaseIndex];
	SelectedChaseLevel = ChaseIndex + 1;
}

// HandleDead()
void AVGMonsterCharacterBase::HandleDead()
{
	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleDead 호출됨: %s"), *GetName());
	//H
	GetWorldTimerManager().ClearTimer(HowlTimerHandle); // 죽으면 하울링 정지
	Multicast_PlaySound(DeathSound);
	
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
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkelMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Deactivate();
	GetCharacterMovement()->SetComponentTickEnabled(false);
	SkelMesh->bPauseAnims = true;
	SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	SkelMesh->SetAllBodiesSimulatePhysics(true);
	SkelMesh->SetPhysicsBlendWeight(1.f);

	for (FBodyInstance* Body : SkelMesh->Bodies)
	{
		if (Body)
		{
			Body->SetCollisionProfileName(TEXT("Ragdoll"));
			Body->SetLinearVelocity(FVector::ZeroVector, false);
			Body->SetAngularVelocityInRadians(FVector::ZeroVector, false);
		}
	}
	SkelMesh->bPauseAnims = true;
}

void AVGMonsterCharacterBase::HandleHit()
{
	if (MonsterAttributeSet->GetHealth() <= 0.f) return;
	
	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleHit 호출됨: %s"), *GetName());
	//H
	Multicast_PlaySound(HitSound);

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
	
}

//H 사운드 재생 본체 (모든 사운드가 여기로 모임)
void AVGMonsterCharacterBase::Multicast_PlaySound_Implementation(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this, Sound, GetActorLocation(), 1.f, 1.f, 0.f, SoundAttenuation);
	}
}

void AVGMonsterCharacterBase::StartHowlTimer()
{
	const float Delay = FMath::FRandRange(HowlIntervalMin, HowlIntervalMax);
	GetWorldTimerManager().SetTimer(
		HowlTimerHandle, this, &AVGMonsterCharacterBase::HandleHowl, Delay, false);
}

void AVGMonsterCharacterBase::HandleHowl()
{
	if (MonsterAttributeSet && MonsterAttributeSet->GetHealth() <= 0.f) return;

	bool bAwake = false;
	if (AIController)
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			bAwake = BB->GetValueAsBool(AVGMonsterAIControllerBase::IsAwakeKey);
		}
	}

	if (!bAwake)
	{
		Multicast_PlaySound(HowlSound);
	}

	StartHowlTimer();
}