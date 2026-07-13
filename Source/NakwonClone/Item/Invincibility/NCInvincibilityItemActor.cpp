// Fill out your copyright notice in the Description page of Project Settings.


#include "NCInvincibilityItemActor.h"


// Sets default values
ANCInvincibilityItemActor::ANCInvincibilityItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANCInvincibilityItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANCInvincibilityItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

