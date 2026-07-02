// Fill out your copyright notice in the Description page of Project Settings.


#include "NCParkinglotGameMode.h"

ANCParkinglotGameMode::ANCParkinglotGameMode()
{
	DefaultPawnClass = nullptr;
}

void ANCParkinglotGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<FName> LevelToPreload;
	LevelToPreload.Add(FName("/Game/Maps/L_Parking"));
	GEngine->PrepareMapChange(GetWorld(), LevelToPreload);
	
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] 지하주차장 프리로딩 중..."));
	
	// 완료 감지용 타이머
	GetWorldTimerManager().SetTimer(
		NextMapReadyTimerHandle,
		this,
		&ANCParkinglotGameMode::CheckMapReady,
		1.f,
		true
	);
}

void ANCParkinglotGameMode::CheckMapReady()
{
	if (GetWorld()->IsMapChangeReady())
	{
		GetWorldTimerManager().ClearTimer(NextMapReadyTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("[NCParkinglotGameMode] 쇼핑몰 프리로딩 완료!"));
	}
}

void ANCParkinglotGameMode::MoveToShopLevel()
{
	if (GetWorld()->IsMapChangeReady())
	{
		GEngine->CommitMapChange(GetWorld());
	}
	else
	{
		GetWorld()->ServerTravel("/Game/Maps/L_ShoppingMall");
		UE_LOG(LogTemp, Warning, TEXT("[NCParkinglotGameMode] 쇼핑몰 프리로딩 실패로 서버트레블 실행"));
	}
}
