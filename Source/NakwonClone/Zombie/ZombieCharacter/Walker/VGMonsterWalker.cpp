// Fill out your copyright notice in the Description page of Project Settings.

#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Common/NCGameplayTags.h"

AVGMonsterWalker::AVGMonsterWalker()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
}

void AVGMonsterWalker::BeginPlay()
{
	Super::BeginPlay();

	// 이동 속도를 BeginPlay에서도 적용 (에디터 값 반영)
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
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
		
			if (TargetASC && !TargetASC->HasMatchingGameplayTag(NCCharacter::Player)) return;
			
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
