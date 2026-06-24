// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_RunSound.h"

#include "Perception/AISense_Hearing.h"

void UAnimNotify_RunSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;
	
	if (APawn* MyPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		UAISense_Hearing::ReportNoiseEvent(
	GetWorld(),
	MyPawn->GetActorLocation(),
	0.25f, // 소음 크기 : 최대 범위 x 1.0f = 청각 범위 (UI 필요)
	MyPawn,
	0.0f);
	}
}
