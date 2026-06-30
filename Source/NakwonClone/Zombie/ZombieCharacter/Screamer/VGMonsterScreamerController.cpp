// Fill out your copyright notice in the Description page of Project Settings.

#include "VGMonsterScreamerController.h"

AVGMonsterScreamerController::AVGMonsterScreamerController()
{
	// base 생성자에서 Perception(시각/청각)이 이미 구성됨.
	// 스크리머는 멀리서도 플레이어를 포착하도록 시야를 더 키우고 싶다면 여기서 조정:
	// if (SightConfig)
	// {
	//     SightConfig->SightRadius     = 1200.f;
	//     SightConfig->LoseSightRadius = 1600.f;
	//     MonsterPerceptionComponent->ConfigureSense(*SightConfig);
	// }
}

void AVGMonsterScreamerController::BeginPlay()
{
	Super::BeginPlay();
}