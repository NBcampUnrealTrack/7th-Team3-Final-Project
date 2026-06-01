// Fill out your copyright notice in the Description page of Project Settings.


#include "NCTitleAndLobbyGameMode.h"

ANCTitleAndLobbyGameMode::ANCTitleAndLobbyGameMode()
{
	DefaultPawnClass = nullptr;
}

void ANCTitleAndLobbyGameMode::MoveToGameLevel()
{
	GetWorld()->ServerTravel("/Game/Maps/L_MVP?listen");
}
