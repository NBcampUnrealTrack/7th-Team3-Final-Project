// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"
#include "NakwonClone/Zombie/AI/AttackSlot/VGAttackSlotComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "BrainComponent.h"
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
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	MonsterAttributeSet = CreateDefaultSubobject<UVGMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
	
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
		
		if (MonsterAttributeSet)
		{
			AbilitySystemComponent->AddSpawnedAttribute(MonsterAttributeSet);
		}
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
	
	// 이동 속도를 BeginPlay에서도 적용 (에디터 값 반영)
	if (GetCharacterMovement() && MonsterAttributeSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = MonsterAttributeSet->GetMoveSpeed();
	}

	if (bRandomType)   // BP에서 켤 수 있는 플래그
	{
		const int32 Count = (int32)EVGMonsterType::MAX;  // 새 타입 추가해도 자동 반영
		MonsterType = (EVGMonsterType)FMath::RandRange(0, Count - 1);
	}
	// 타입 데이터로 외형/스탯/공격 세팅 (랜덤메시 대체)
	ApplyMonsterType();
	
	if (AnimMove.Num() > 0)
	{
		int32 MoveIndex = FMath::RandRange(0, AnimMove.Num() - 1);
		SelectedMoveMontage = AnimMove[MoveIndex];
		SelectedMoveLevel = MoveIndex + 1;
	}

	if (AnimChase.Num() > 0)
	{
		int32 ChaseIndex = FMath::RandRange(0, AnimChase.Num() - 1);
		SelectedChaseMontage = AnimChase[ChaseIndex];
		SelectedChaseLevel = ChaseIndex + 1;
	}

	SelectedStopMontage = GetRandomStopMontage();

	DetectionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AVGMonsterCharacterBase::OnDetectionOverlap);

	// H
	if (HasAuthority())
	{
		StartHowlTimer();
		StartIdleTimer();
	}
}

// HandleDead()
void AVGMonsterCharacterBase::HandleDead()
{
	if (bIsDead) return;   // 사망 처리도 한 번만
	bIsDead = true;

	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleDead 호출됨: %s"), *GetName());
	
	//H
	GetWorldTimerManager().ClearTimer(HowlTimerHandle); // 죽으면 하울링 정지
	Multicast_PlaySound(DeathSound, CombatAttenuation);
	
	// 레그돌
	OnStartRagdoll();

	OnStartDissolve();
	
	ReleaseAttackSlot();

	if (AIController)
	{
		// [BT 리팩터] IsDeadKey 폐기 — UnPossess로 BT 정지 처리, 아래 키 세팅만 주석
		//if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		//{
		//	Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsDeadKey, true);
		//}
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
	if (bIsDead || bIsBeingAssassinated || MonsterAttributeSet->GetHealth() <= 0.f) return;
	// 피격 시에도 큰소리(스페셜) 재생 — "총 맞아도 운다"
	TryPlaySpecialMontage();
	
	const EVGHitBodyPart BodyPart = HitData.BodyPart;

	UE_LOG(LogMonster, Warning, TEXT("[MonsterBase] HandleHit: 부위=%d"),
		static_cast<int32>(BodyPart));

	// 사운드 (부위별, 없으면 기본 HitSound)  //H
	if (USoundBase* Sound = GetHitSoundByPart(BodyPart))
	{
		Multicast_PlaySound(Sound, CombatAttenuation);
	}

	// VFX (부위별, 타격 위치에)
	if (UNiagaraSystem* VFX = GetHitVFXByPart(BodyPart))
	{
		const FVector Loc = HitData.HitLocation.IsNearlyZero()
			? GetMesh()->GetComponentLocation()
			: HitData.HitLocation;
		Multicast_SpawnHitVFX(VFX, Loc);
	}
	
	// 부위별 피격 몽타주 (확률적으로만 재생 = 넉백)
	if (FMath::FRand() <= HitReactChance)
	{
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
	}
	
	// 피격 GameplayCue (혈흔 VFX + 데칼)
	if (AbilitySystemComponent)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitData.HitLocation;
		CueParams.Normal   = HitData.HitNormal;
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
void AVGMonsterCharacterBase::Multicast_PlaySound_Implementation(USoundBase* Sound, USoundAttenuation* AttenuationOverride)
{
	if (!Sound) return;
	USoundAttenuation* Atten = AttenuationOverride ? AttenuationOverride : SoundAttenuation.Get();
	UGameplayStatics::PlaySoundAtLocation(
		this, Sound, GetActorLocation(), 1.f, 1.f, 0.f, Atten);
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
	// [BT 리팩터] IsAwakeKey 폐기 — 사운드(HowlSound) 자체는 유지, 키 참조만 주석
	//if (AIController)
	//{
	//	if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
	//	{
	//		bAwake = BB->GetValueAsBool(AVGMonsterAIControllerBase::IsAwakeKey);
	//	}
	//}

	if (!bAwake)
	{
		Multicast_PlaySound(HowlSound, CombatAttenuation);
	}

	StartHowlTimer();
}

void AVGMonsterCharacterBase::StartIdleTimer()
{
	if (!HasAuthority() || !IdleSound) return;

	const float Delay = IdleSound->GetDuration() + IdleSoundCooldown;
	GetWorldTimerManager().SetTimer(
		IdleTimerHandle, this, &AVGMonsterCharacterBase::HandleIdle, Delay, false);
}

void AVGMonsterCharacterBase::HandleIdle()
{
	if (bIsDead) return;   // 죽으면 멈춤 (재예약 안 함)

	Multicast_PlaySound(IdleSound, CombatAttenuation);
	StartIdleTimer();      // 다음 주기 예약
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

void AVGMonsterCharacterBase::BeginAssassinationVictim(AActor* Killer, UAnimMontage* VictimMontage)
{
	if (bIsDead || bIsBeingAssassinated) return;
	bIsBeingAssassinated = true;

	// (1) 즉시 정지 — 이동 차단
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// (2) AI 두뇌 정지 (BT가 새 몽타주 못 끼게)
	if (AIController)
	{
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
			Brain->StopLogic(TEXT("Assassinated"));
		AIController->StopMovement();
	}

	// (3) 구도 정렬 — 좀비 등이 플레이어를 향하도록 스냅
	if (Killer)
	{
		const FVector KillerFwd = Killer->GetActorForwardVector();
		SetActorLocation(Killer->GetActorLocation() + KillerFwd * AssassinationAlignDistance);
		SetActorRotation(KillerFwd.Rotation());
	}

	// (4) 재생중인 일반 몽타주 끊기
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		Anim->StopAllMontages(0.1f);

	// (5) victim 몽타주 동기 재생 (DT에서 받은 것)
	if (VictimMontage)
		Multicast_PlayAssassinationMontage(VictimMontage);
}

void AVGMonsterCharacterBase::Multicast_PlayAssassinationMontage_Implementation(UAnimMontage* Montage)
{
	if (!Montage) return;
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		Anim->Montage_Play(Montage);
}

void AVGMonsterCharacterBase::ApplyMonsterType()
{
	if (!MonsterTypeTable) return;

	// enum 이름 → 행 이름 ("EVGMonsterType::Walker" → "Walker")
	FString EnumStr = UEnum::GetValueAsString(MonsterType);
	FString RowStr;
	EnumStr.Split(TEXT("::"), nullptr, &RowStr);
	const FName RowName(*RowStr);

	FVGMonsterTypeRow* Row = MonsterTypeTable->FindRow<FVGMonsterTypeRow>(RowName, TEXT("ApplyMonsterType"));
	if (!Row) return;

	// 공격/특수 몽타주 캐시
	CachedAttackMontages = Row->AttackMontages;
	CachedSpecialMontage = Row->SpecialMontage;

	CachedLocomotionBS = Row->LocomotionBS;

	// 외형
	if (Row->Mesh)      GetMesh()->SetSkeletalMesh(Row->Mesh);
	if (Row->AnimClass) GetMesh()->SetAnimInstanceClass(Row->AnimClass);
	SetActorScale3D(FVector(Row->MeshScale));

	// 스탯 (Tank는 Health 크게)
	if (MonsterAttributeSet)
	{
		MonsterAttributeSet->InitHealth(Row->MaxHealth);
		MonsterAttributeSet->InitMoveSpeed(Row->MoveSpeed);
	}
	
	CachedPatrolSpeed = Row->PatrolSpeed;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Row->PatrolSpeed;
	}
	
	CachedAttackEffectClass = Row->AttackEffectClass;

	if (Row->HitSound)                  HitSound = Row->HitSound;
	if (Row->HitSoundsByPart.Num() > 0) HitSoundsByPart = Row->HitSoundsByPart;

	HitReactChance = Row->HitReactChance; 
}

UAnimMontage* AVGMonsterCharacterBase::GetAttackMontageForAI()
{
	if (CachedAttackMontages.Num() > 0)
		return GetRandomMontage(CachedAttackMontages);
	return nullptr;
}

void AVGMonsterCharacterBase::TryPlaySpecialMontage()
{
	// 스페셜 몽타주 없는 타입(Walker/Tank)은 자동 무시 → 타입 분기 불필요
	if (!CachedSpecialMontage) return;
	if (bIsDead || bIsBeingAssassinated) return;

	// 쿨다운: 접촉 + 연속 피격이 겹쳐도 한 번씩만
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastSpecialMontageTime < SpecialMontageCooldown) return;
	LastSpecialMontageTime = Now;

	Multicast_PlaySpecialMontage(CachedSpecialMontage);
}

void AVGMonsterCharacterBase::Multicast_PlaySpecialMontage_Implementation(UAnimMontage* Montage)
{
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (Montage) Anim->Montage_Play(Montage);
	}
}

void AVGMonsterCharacterBase::StartAttack()
{
	// Witch: 첫 공격이면 큰소리 먼저 → 끝나면 공격
	if (MonsterType == EVGMonsterType::Witch && !bHasScreamed && CachedSpecialMontage)
	{
		bScreamPhase = true;
		CurrentPlayingMontage = CachedSpecialMontage;
		Multicast_PlayAttackMontage(CachedSpecialMontage);

		if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			Anim->OnMontageEnded.RemoveDynamic(this, &AVGMonsterCharacterBase::OnAttackMontageEnded);
			Anim->OnMontageEnded.AddDynamic(this, &AVGMonsterCharacterBase::OnAttackMontageEnded);
		}
		return;
	}

	// Walker / Tank / (Witch 큰소리 이후) → 바로 공격
	PlayAttackNow();
}

void AVGMonsterCharacterBase::PlayAttackNow()
{
	UAnimMontage* Montage = GetAttackMontageForAI();
	if (!Montage)
	{
		OnAttackFinished.ExecuteIfBound();   // 공격 몽타주 없으면 즉시 완료
		return;
	}

	bScreamPhase = false;
	CurrentPlayingMontage = Montage;
	Multicast_PlayAttackMontage(Montage);

	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		Anim->OnMontageEnded.RemoveDynamic(this, &AVGMonsterCharacterBase::OnAttackMontageEnded);
		Anim->OnMontageEnded.AddDynamic(this, &AVGMonsterCharacterBase::OnAttackMontageEnded);
	}
}

void AVGMonsterCharacterBase::Multicast_PlayAttackMontage_Implementation(UAnimMontage* Montage)
{
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (Montage) Anim->Montage_Play(Montage);
	}
}

void AVGMonsterCharacterBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentPlayingMontage) return;

	// Witch 큰소리가 끝난 거면 → 이제 진짜 공격
	if (bScreamPhase)
	{
		bScreamPhase = false;
		bHasScreamed = true;
		PlayAttackNow();     // 아직 OnAttackFinished 안 쏨 (공격까지 기다림)
		return;
	}

	// 공격이 끝난 거면 → BT에 알림
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		Anim->OnMontageEnded.RemoveDynamic(this, &AVGMonsterCharacterBase::OnAttackMontageEnded);
	}
	CurrentPlayingMontage = nullptr;
	OnAttackFinished.ExecuteIfBound();
}

bool AVGMonsterCharacterBase::ReserveAttackSlot(AActor* Target, FVector& OutSlotLocation)
{
	if (!Target) return false;
	UVGAttackSlotComponent* SlotComp = Target->FindComponentByClass<UVGAttackSlotComponent>();
	if (!SlotComp) { SlotComp = NewObject<UVGAttackSlotComponent>(Target); SlotComp->RegisterComponent(); }

	if (ReservedSlotComp.Get() && ReservedSlotComp.Get() != SlotComp)
	{
		ReleaseAttackSlot();
	}
	else if (ReservedSlotComp.Get() == SlotComp && bReservedIsWaitSlot)
	{
		// 같은 대상, Wait → Attack 승급: 기존 Wait Slot 반납
		SlotComp->ReleaseWaitSlot(this);
	}

	int32 SlotIndex;
	if (!SlotComp->RequestSlot(this, SlotIndex)) return false;

	ReservedSlotComp = SlotComp;
	ReservedSlotIndex = SlotIndex;
	bReservedIsWaitSlot = false;
	OutSlotLocation = SlotComp->GetSlotLocation(SlotIndex);
	return true;
}

void AVGMonsterCharacterBase::ReleaseAttackSlot()
{
	if (UVGAttackSlotComponent* SlotComp = ReservedSlotComp.Get())
	{
		if (bReservedIsWaitSlot) SlotComp->ReleaseWaitSlot(this);
		else SlotComp->ReleaseSlot(this);
	}
	ReservedSlotComp = nullptr;
	ReservedSlotIndex = -1;
	bReservedIsWaitSlot = false;
}

bool AVGMonsterCharacterBase::GetReservedSlotLocation(FVector& OutLocation) const
{
	UVGAttackSlotComponent* SlotComp = ReservedSlotComp.Get();
	if (!SlotComp || ReservedSlotIndex == -1) return false;
	OutLocation = bReservedIsWaitSlot ? SlotComp->GetWaitSlotLocation(ReservedSlotIndex) : SlotComp->GetSlotLocation(ReservedSlotIndex);
	return true;
}

bool AVGMonsterCharacterBase::ReserveWaitSlot(AActor* Target, FVector& OutSlotLocation)
{
	if (!Target) return false;
	UVGAttackSlotComponent* SlotComp = Target->FindComponentByClass<UVGAttackSlotComponent>();
	if (!SlotComp) { SlotComp = NewObject<UVGAttackSlotComponent>(Target); SlotComp->RegisterComponent(); }

	if (ReservedSlotComp.Get() && ReservedSlotComp.Get() != SlotComp)
	{
		ReleaseAttackSlot();
	}
	else if (ReservedSlotComp.Get() == SlotComp && !bReservedIsWaitSlot)
	{
		SlotComp->ReleaseSlot(this);
	}

	int32 SlotIndex;
	if (!SlotComp->RequestWaitSlot(this, SlotIndex)) return false;

	ReservedSlotComp = SlotComp;
	ReservedSlotIndex = SlotIndex;
	bReservedIsWaitSlot = true;
	OutSlotLocation = SlotComp->GetWaitSlotLocation(SlotIndex);
	return true;
}