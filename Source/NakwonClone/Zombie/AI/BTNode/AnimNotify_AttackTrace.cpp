// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackTrace.h"
#include "NakwonClone/Zombie/ZombieCharacter/Walker/VGMonsterWalker.h"


void UAnimNotify_AttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;
	
	AVGMonsterWalker* Walker = Cast<AVGMonsterWalker>(MeshComp->GetOwner());
	if (!Walker) return;
	
	Walker->PerformAttackTrace();
}
