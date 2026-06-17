// Fill out your copyright notice in the Description page of Project Settings.


#include "NCTitleAndLobbyGameMode.h"

#include "Framwork/GameInstacne/NCGameInstance.h"
#include "Kismet/GameplayStatics.h"

ANCTitleAndLobbyGameMode::ANCTitleAndLobbyGameMode()
{
	DefaultPawnClass = nullptr;
}

void ANCTitleAndLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("TitleAndLobbyGameMode BeginPlay 호출됨"));
	
	UNCGameInstance* GI = Cast<UNCGameInstance>(GetGameInstance());
	if (!GI) return;
	GI->StartPreloading();
	UE_LOG(LogTemp, Warning, TEXT("StartPreloading 호출됨"));
}

void ANCTitleAndLobbyGameMode::MoveToGameLevel()
{
	GetWorld()->ServerTravel("/Game/Maps/L_MVP?listen");
}
