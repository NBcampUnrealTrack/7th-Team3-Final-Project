// Fill out your copyright notice in the Description page of Project Settings.


#include "VGMonsterWalkerController.h"
#include "BehaviorTree/BlackboardComponent.h"


//헌호수정 - 부모의 Detour Crowd 생성자에 FObjectInitializer 전달
AVGMonsterWalkerController::AVGMonsterWalkerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bCanSleep = true;
}

void AVGMonsterWalkerController::BeginPlay()
{
	Super::BeginPlay();
}

