// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"
#include "NakwonClone/Zombie/ZombieCharacter/Runner/VGMonsterRunner.h"
#include "GameplayEffect.h"
#include "Common/NCGameplayTags.h"
#include "Engine/OverlapResult.h"

void UAnimNotify_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	HitActors.Empty();
}

void UAnimNotify_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	DoHitCheck(MeshComp);
}

void UAnimNotify_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UAnimNotify_AttackTrace::DoHitCheck(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp) return;
	
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(MeshComp->GetOwner());
	if (!Walker) return;
	
	UWorld* World = Walker->GetWorld();
	if (!World) return;
	
	if (!Walker->AttackEffectClass) return;
	
	UAbilitySystemComponent* ASC = Walker->GetAbilitySystemComponent();
	if (!ASC) return;
	
	const float Radius = Walker->GetAttackTraceDistance();
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Walker);
	
	for (const FName& SocketName : Walker->GetAttackSocketNames())
	{
		if (!MeshComp->DoesSocketExist(SocketName)) continue;
		
		const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
		
		TArray<FOverlapResult> Overlaps;
		bool bHit = World->OverlapMultiByChannel(
			Overlaps,
			SocketLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Radius),
			Params);
		
#if WITH_EDITOR
		DrawDebugSphere(World, 
			SocketLocation, 
			Radius, 
			8,
			bHit ? FColor::Red : FColor::Green,
			false,
			0.2f);
	#endif
		
		if (!bHit) continue;
		
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* HitActor = Overlap.GetActor();
			if (!HitActor) return;
			if (HitActors.Contains(HitActor)) continue;
			
			UAbilitySystemComponent* TargetASC =
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
			if (!TargetASC) return;
			if (!TargetASC->HasMatchingGameplayTag(NCCharacter::Player)) continue;
			
			HitActors.Add(HitActor);
			
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			
			FHitResult AttackHit;
			AttackHit.Location     = SocketLocation;
			AttackHit.ImpactPoint  = SocketLocation;
			AttackHit.ImpactNormal = (HitActor->GetActorLocation() - SocketLocation).GetSafeNormal();
			ContextHandle.AddHitResult(AttackHit);
			
			FGameplayEffectSpecHandle Spec =
				ASC->MakeOutgoingSpec(Walker->AttackEffectClass, 1.f, ContextHandle);
			
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
			}
		}
	}
}