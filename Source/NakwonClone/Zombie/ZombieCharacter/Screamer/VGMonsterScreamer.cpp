// Fill out your copyright notice in the Description page of Project Settings.

#include "VGMonsterScreamer.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

AVGMonsterScreamer::AVGMonsterScreamer()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
}

void AVGMonsterScreamer::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterMovement() && MonsterAttributeSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = MonsterAttributeSet->GetMoveSpeed();
	}

	if (RandomMesh.Num() > 0)
	{
		const int32 RandIndex = FMath::RandRange(0, RandomMesh.Num() - 1);
		GetMesh()->SetSkeletalMesh(RandomMesh[RandIndex]);
	}

	if (HasAuthority())
	{

		if (AIController)
		{
			if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AVGMonsterAIControllerBase::IsAwakeKey, true);
			}
		}

		// BT가 비명을 안 몰면(아직 BTTask_Scream 미연결) 자동 루프로라도 동작
		if (bAutoScream)
		{
			GetWorldTimerManager().SetTimer(
				AutoScreamTimer, this, &AVGMonsterScreamer::AutoScreamCheck,
				AutoScreamCheckInterval, true);
		}
	}
}

bool AVGMonsterScreamer::CanScream() const
{
	if (bScreamOnCooldown) return false;
	if (IsDead() || IsBeingAssassinated()) return false;  
	if (MonsterAttributeSet && MonsterAttributeSet->GetHealth() <= 0.f) return false;
	return true;
}

void AVGMonsterScreamer::RequestScream(AActor* TargetActor)
{
	if (!HasAuthority()) return; 
	if (!CanScream()) return;

	PerformScream(TargetActor);
}

void AVGMonsterScreamer::PerformScream(AActor* TargetActor)
{
	if (!HasAuthority() || !CanScream()) return;

	bScreamOnCooldown = true;

	UAnimMontage* ScreamMontage = GetRandomMontage(AnimScream);
	if (ScreamMontage)
	{
		Multicast_PlayScreamMontage(ScreamMontage);
	}
	Multicast_PlaySound(ScreamSound ? ScreamSound : HowlSound);


	if (TargetActor)
	{
		UAISense_Hearing::ReportNoiseEvent(
			GetWorld(),
			TargetActor->GetActorLocation(), 
			ScreamLoudness,
			TargetActor,                  
			ScreamRadius);
	}

	// (3) 쿨다운
	UE_LOG(LogMonster, Warning, TEXT("[Screamer] 비명! target=%s radius=%.0f"),
		TargetActor ? *TargetActor->GetName() : TEXT("None"), ScreamRadius);

	GetWorldTimerManager().SetTimer(
		ScreamCooldownTimer, this, &AVGMonsterScreamer::OnScreamCooldownElapsed,
		ScreamCooldown, false);
}

void AVGMonsterScreamer::Multicast_PlayScreamMontage_Implementation(UAnimMontage* Montage)
{
	if (!Montage) return;
	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		Anim->Montage_Play(Montage);
	}
}

void AVGMonsterScreamer::OnScreamCooldownElapsed()
{
	bScreamOnCooldown = false;
}

void AVGMonsterScreamer::AutoScreamCheck()
{
	if (!HasAuthority() || !CanScream()) return;
	if (!AIController) return;

	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB) return;

	// 시야에 플레이어가 잡혀 있으면(=TargetActor) 비명.
	if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey)))
	{
		PerformScream(Target);
	}
}