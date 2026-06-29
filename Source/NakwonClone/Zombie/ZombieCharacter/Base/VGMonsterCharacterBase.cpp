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
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
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

	// UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] BeginPlay 호출됨: %s"), *GetName());

	AIController = Cast<AVGMonsterAIControllerBase>(GetController());

	if (!AIController)
	{
		// UE_LOG(LogMonster, Error, TEXT("[MonsterBase] AIController 캐스팅 실패: %s"), *GetName());
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

	int32 MoveIndex = FMath::RandRange(0, AnimMove.Num() - 1);
	SelectedMoveMontage = AnimMove[MoveIndex];
	SelectedMoveLevel = MoveIndex + 1;

	int32 ChaseIndex = FMath::RandRange(0, AnimChase.Num() - 1);
	SelectedChaseMontage = AnimChase[ChaseIndex];
	SelectedChaseLevel = ChaseIndex + 1;
}

// HandleDead()
void AVGMonsterCharacterBase::HandleDead()
{
	if (bIsDead) return;   // 사망 처리도 한 번만
	bIsDead = true;

	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleDead 호출됨: %s"), *GetName());
	
	if (bIsDead) return;
	bIsDead = true;
	
	//H
	GetWorldTimerManager().ClearTimer(HowlTimerHandle); // 죽으면 하울링 정지
	Multicast_PlaySound(DeathSound);
	
	// 레그돌
	OnStartRagdoll();

	OnStartDissolve();

	if (AIController)
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsDeadKey, true);
		}
		AIController->StopMovement();
		AIController->UnPossess();
	}
}

void AVGMonsterCharacterBase::OnStartRagdoll()
{
	USkeletalMeshComponent* SkelMesh = GetMesh(); 
	if (!SkelMesh) return;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetectionCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//SkelMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->Deactivate();
	GetCharacterMovement()->SetComponentTickEnabled(false);
	SkelMesh->bPauseAnims = true;
	SkelMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	SkelMesh->SetAllBodiesSimulatePhysics(true);
	SkelMesh->SetPhysicsBlendWeight(1.f);
	SkelMesh->BodyInstance.SetLinearVelocity(FVector::ZeroVector, false);
	SkelMesh->BodyInstance.SetAngularVelocityInRadians(FVector::ZeroVector, false);

	/*for (FBodyInstance* Body : SkelMesh->Bodies)
	{
		if (Body)
		{
			Body->SetCollisionProfileName(TEXT("Ragdoll"));
			Body->SetLinearVelocity(FVector::ZeroVector, false);
			Body->SetAngularVelocityInRadians(FVector::ZeroVector, false);
		}
	}*/
}

void AVGMonsterCharacterBase::HandleHit(const FVGHitData& HitData)
{
	if (bIsDead || MonsterAttributeSet->GetHealth() <= 0.f) return;
	
	const EVGHitBodyPart BodyPart = HitData.BodyPart;

	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleHit: 부위=%d"),
		static_cast<int32>(BodyPart));

	// 사운드 (부위별, 없으면 기본 HitSound)  //H
	if (USoundBase* Sound = GetHitSoundByPart(BodyPart))
	{
		Multicast_PlaySound(Sound);
	}

	// VFX (부위별, 타격 위치에)
	if (UNiagaraSystem* VFX = GetHitVFXByPart(BodyPart))
	{
		const FVector Loc = HitData.HitLocation.IsNearlyZero()
			? GetMesh()->GetComponentLocation()
			: HitData.HitLocation;
		Multicast_SpawnHitVFX(VFX, Loc);
	}

	// AI '맞는 중' 신호
	if (AIController)
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsHitKey, true);
		}
	}

	// 부위별 피격 몽타주 (무조건 끊고 새로)
	PendingHitBodyPart = BodyPart;
	GetWorldTimerManager().ClearTimer(HitReactTimerHandle);
	if (HitReactDelay <= 0.f)
	{
		PlayHitReactMontage(BodyPart);
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			HitReactTimerHandle, this,
			&AVGMonsterCharacterBase::OnHitReactDelayElapsed, HitReactDelay, false);
	}

	// GC 호출
	if (AbilitySystemComponent)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitData.HitLocation;
		CueParams.Normal = HitData.HitNormal;  // FVGHitData에 Normal 있으면
		AbilitySystemComponent->ExecuteGameplayCue(
			FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit.Zombie")),
			CueParams);
	}

	/*// 뒤로 밀려남
	FVector PushBack = -GetActorForwardVector();
	LaunchCharacter(PushBack * 300.f, true, false);*/
}

UAnimMontage* AVGMonsterCharacterBase::GetRandomHitMontageByPart(EVGHitBodyPart BodyPart)
{
	// 1순위: 해당 부위 몽타주
	if (const FVGHitMontageList* List = HitMontagesByPart.Find(BodyPart))
	{
		if (List->Montages.Num() > 0)
		{
			return GetRandomMontage(List->Montages);
		}
	}
	// 2순위: None 부위에 넣어둔 공용 몽타주
	if (const FVGHitMontageList* NoneList = HitMontagesByPart.Find(EVGHitBodyPart::None))
	{
		if (NoneList->Montages.Num() > 0)
		{
			return GetRandomMontage(NoneList->Montages);
		}
	}
	// 3순위 폴백: 기존 AnimHit 배열 (에셋 아직 안 꽂았을 때)
	return GetRandomHitMontage();
}

void AVGMonsterCharacterBase::PlayHitReactMontage(EVGHitBodyPart BodyPart)
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	UAnimInstance* Anim = MeshComp ? MeshComp->GetAnimInstance() : nullptr;
	if (!Anim) return;

	UAnimMontage* Montage = GetRandomHitMontageByPart(BodyPart);
	if (!Montage)
	{
		UE_LOG(LogMonster, Warning,
			TEXT("[MonsterBase] 부위(%d) 피격 몽타주 없음 — 에셋 미설정"),
			static_cast<int32>(BodyPart));
		return;
	}

	// 무조건 끊고 새로: 재생 중이던 피격 몽타주를 짧게 블렌드아웃
	if (CurrentHitMontage && Anim->Montage_IsPlaying(CurrentHitMontage))
	{
		Anim->Montage_Stop(HitReactBlendOutTime, CurrentHitMontage);
	}

	Anim->Montage_Play(Montage);
	CurrentHitMontage = Montage;

	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] 피격 몽타주 재생: 부위=%d"),
		static_cast<int32>(BodyPart));
}

void AVGMonsterCharacterBase::OnHitReactDelayElapsed()
{
	PlayHitReactMontage(PendingHitBodyPart);
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

USoundBase* AVGMonsterCharacterBase::GetHitSoundByPart(EVGHitBodyPart BodyPart) const
{
	if (const TObjectPtr<USoundBase>* Found = HitSoundsByPart.Find(BodyPart))
	{
		if (*Found) return *Found;
	}
	return HitSound;
}

UNiagaraSystem* AVGMonsterCharacterBase::GetHitVFXByPart(EVGHitBodyPart BodyPart) const
{
	if (const TObjectPtr<UNiagaraSystem>* Found = HitVFXByPart.Find(BodyPart))
	{
		return *Found;
	}
	return nullptr;
}

void AVGMonsterCharacterBase::Multicast_SpawnHitVFX_Implementation(
	UNiagaraSystem* VFX, FVector Location)
{
	if (!VFX) return;
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, VFX, Location);
}