// Fill out your copyright notice in the Description page of Project Settings.


#include "NCGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerController/NCPlayerController.h"

ANCGameState::ANCGameState()
{
	WalkerKillCount = 0;
	RunnerKillCount = 0;
	WitchKillCount = 0;
	TankKillCount = 0;
}

void ANCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCGameState, ConnectedPlayerCount);
	DOREPLIFETIME(ANCGameState, AlivePlayerCount);
	DOREPLIFETIME(ANCGameState, CurrentGameStateTag);
	DOREPLIFETIME(ANCGameState, RemainingMatchTime);
	DOREPLIFETIME(ANCGameState, TotalScore);
	DOREPLIFETIME(ANCGameState, bEscapable);

	//헌호수정 - 타입별 킬 카운트 복제 (게임 종료 점수판, 멀티 클라 표시용)
	DOREPLIFETIME(ANCGameState, WalkerKillCount);
	DOREPLIFETIME(ANCGameState, RunnerKillCount);
	DOREPLIFETIME(ANCGameState, WitchKillCount);
	DOREPLIFETIME(ANCGameState, TankKillCount);
}

void ANCGameState::OnRep_TotalScore(int32 OldTotalScore)
{
	OnScoreChanged.Broadcast(TotalScore, TotalScore - OldTotalScore);
}

void ANCGameState::OnRep_bEscapable()
{
	if (!bEscapable || !ClearWidgetClass)
	{
		return;
	}
	
	ANCPlayerController* PC = Cast<ANCPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		if (UUserWidget* ClearWidget = CreateWidget<UUserWidget>(PC, ClearWidgetClass))
		{
			ClearWidget->AddToViewport();
		}
	}
	// todo : 탈출 가능 안내 위젯 바인딩
}

void ANCGameState::Multicast_PlayHelicopterSound_Implementation(USoundBase* Sound)
{
	UGameplayStatics::PlaySound2D(this, Sound);
}

void ANCGameState::OnRep_CurrentGameStateTag()
{
	// todo : UI갱신?
}

void ANCGameState::OnRep_MatchTime()
{
	// todo : 타이머 UI 갱신
	OnRemainingMatchTimeUpdate.Broadcast(RemainingMatchTime);
}
