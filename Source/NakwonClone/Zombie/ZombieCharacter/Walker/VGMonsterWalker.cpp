// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/NCGameplayTags.h"
#include "NakwonClone\GAS\AttributeSet\VGMonsterAttributeSet.h"
#include "Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

AVGMonsterWalker::AVGMonsterWalker()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
}

void AVGMonsterWalker::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도를 BeginPlay에서도 적용 (에디터 값 반영)
	if (GetCharacterMovement() && MonsterAttributeSet)
	{
		GetCharacterMovement()->MaxWalkSpeed = MonsterAttributeSet->GetMoveSpeed();
	}
	
	if (AnimWakeUp.Num() > 0)
	{
		SelectedWakeUpMontage = AnimWakeUp[FMath::RandRange(0, AnimWakeUp.Num() - 1)];
	}
	
	if (RandomMesh.Num() > 0)
	{
		int32 RandIndex = FMath::RandRange(0, RandomMesh.Num() - 1);
		GetMesh()->SetSkeletalMesh(RandomMesh[RandIndex]);
		/*UE_LOG(LogMonster, Warning, TEXT("[Mesh] Physics Asset: %s"), 
		GetMesh()->GetPhysicsAsset() ? *GetMesh()->GetPhysicsAsset()->GetName() : TEXT("None"));*/
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

