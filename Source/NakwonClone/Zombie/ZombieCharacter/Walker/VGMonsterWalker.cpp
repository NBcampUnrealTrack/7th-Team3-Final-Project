// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
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
	
	if (MonsterAttributeSet)
	{
		// 피격 이벤트 오면 HandleHit 실행
		MonsterAttributeSet->OnHitReceived.AddDynamic(this, &AVGMonsterWalker::HandleHit);
		// 사망 이벤트 오면 HandleDead 실행
		MonsterAttributeSet->OnDead.AddDynamic(this, &AVGMonsterWalker::HandleDead);
	}
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

void AVGMonsterWalker::HandleHit()
{
	if (MonsterAttributeSet->GetHealth() <= 0.f) return;
	SetMonsterState(EMonsterState::Hit);
}

void AVGMonsterWalker::HandleDead()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}

	SetActorEnableCollision(false);

	float Duration = PlayAnimMontage(AnimDead);

	FTimerHandle FreezeHandle;
	GetWorldTimerManager().SetTimer(FreezeHandle, [this]()
	{
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Pause(AnimDead);
		}
	}, 1.16f, false);

	SetLifeSpan(200.f);
}

void AVGMonsterWalker::PerformAttackTrace()
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (!SkeletalMesh) return;
	
	for (const FName& SocketName : AttackSocketNames)
	{
		FVector SocketLocation = SkeletalMesh->GetSocketLocation(SocketName);
		FVector TraceEnd = SocketLocation + GetActorForwardVector() * AttackTraceDistance;
		
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
	
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			SocketLocation,
			TraceEnd,
			ECC_Pawn,
			Params);
		
		DrawDebugLine(GetWorld(), SocketLocation, TraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.f);
	
		if (bHit)
		{
			UAbilitySystemComponent* TargetASC = 
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit.GetActor());
			UAbilitySystemComponent* MonsterASC = 
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(this);
		
			if (TargetASC && MonsterASC && AttackEffectClass)
			{
				FGameplayEffectContextHandle EffectContext = MonsterASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);
				FGameplayEffectSpecHandle  SpecHandle = MonsterASC->MakeOutgoingSpec(
					AttackEffectClass, 2.f, EffectContext);
			
				if (SpecHandle.IsValid())
				{
					MonsterASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
			return;
		}
		
	}
}
