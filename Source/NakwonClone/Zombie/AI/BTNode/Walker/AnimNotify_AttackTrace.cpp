// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackTrace.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"

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
	
}
