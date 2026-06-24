// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_WalkSound.h"

#include "Perception/AISense_Hearing.h"

void UAnimNotify_WalkSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;
	
	if (APawn* MyPawn = Cast<APawn>(MeshComp->GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WalkSound] Notify 호출됨"));
		UAISense_Hearing::ReportNoiseEvent(
			GetWorld(),
			MyPawn->GetActorLocation(),
			0.05f, // 소음 크기 : 최대 범위 x 1.0f = 청각 범위 (UI 필요)
			MyPawn,
			0.0f);
	}
}
