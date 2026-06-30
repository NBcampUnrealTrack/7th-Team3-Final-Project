// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/NCGameplayTags.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

AVGMonsterWalker::AVGMonsterWalker()
{
}

void AVGMonsterWalker::BeginPlay()
{
	Super::BeginPlay();
	
	if (AnimWakeUp.Num() > 0)
	{
		SelectedWakeUpMontage = AnimWakeUp[FMath::RandRange(0, AnimWakeUp.Num() - 1)];
	}
}

void AVGMonsterWalker::OnDetectionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!AIController) return;
	
	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC || !TargetASC->HasMatchingGameplayTag(NCCharacter::Player)) return;
	
	WakeUpWithDelay();
}

void AVGMonsterWalker::WakeUpWithDelay()
{
	float Delay = FMath::RandRange(0.f, 2.f);
	GetWorldTimerManager().SetTimer(WakeUpTimerHandle, this, &AVGMonsterWalker::WakeUp, Delay, false);
}

void AVGMonsterWalker::WakeUp()
{
	if (!AIController) return;
	if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
	{
		//H 아직 안 깨어났을 때만 발견 사운드
		if (!Blackboard->GetValueAsBool(AVGMonsterAIControllerBase::IsAwakeKey))
		{
			Multicast_PlaySound(DetectSound);
		}
		// 몬스터 기상
		Blackboard->SetValueAsBool(AVGMonsterAIControllerBase::IsAwakeKey, true);
	}
}

