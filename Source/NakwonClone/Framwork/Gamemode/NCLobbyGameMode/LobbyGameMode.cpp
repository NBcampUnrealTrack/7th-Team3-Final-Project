// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
	DefaultPawnClass = nullptr;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::MoveToShopLevel()
{
	GetWorld()->ServerTravel("/Game/Maps/L_MVP");
}
