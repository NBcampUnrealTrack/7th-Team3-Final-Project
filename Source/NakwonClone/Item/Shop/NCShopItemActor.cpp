// Fill out your copyright notice in the Description page of Project Settings.


#include "NCShopItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Framwork/GameState/NCGameState.h"
#include "Player/PlayerController/NCPlayerController.h"

void ANCShopItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	ANCGameState* GS = GetWorld()->GetGameState<ANCGameState>();
	if (!GS) return;
	
	ANCPlayerController* NCPC = Cast<ANCPlayerController>(Player->GetController());
	if (!NCPC) return;
	
	// 점수 부족
	if (GS->TotalScore < RequiredScore)
	{
		NCPC->Client_ShowNotification(FText::FromString(TEXT("요구 점수가 부족합니다")), FLinearColor::Red);
		return;
	}

	const int32 OldScore = GS->TotalScore;
	GS->TotalScore -= RequiredScore;
	GS->OnRep_TotalScore(OldScore); // 서버에서 직접 수정 시 수동 브로드캐스트 필요 (NCGameMode 기존 패턴과 동일)

	OnPurchased(Player);

	ConsumeItem(Interactor);
}
