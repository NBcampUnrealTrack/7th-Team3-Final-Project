// Fill out your copyright notice in the Description page of Project Settings.

#include "NCInfiniteAmmoItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"
#include "Player/PlayerComponent/NCGunComponent.h"
#include "Player/PlayerController/NCPlayerController.h"

void ANCInfiniteAmmoItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
	if (!Player) return;

	UNCEquipmentComponent* EquipComp = Player->GetEquipmentComponent();
	if (!EquipComp) return;

	ANCPlayerController* NCPC = Cast<ANCPlayerController>(Player->GetController());

	UNCGunComponent* ActiveWeapon = EquipComp->GetActiveWeapon();
	if (!ActiveWeapon)
	{
		if (NCPC)
		{
			NCPC->Client_ShowNotification(FText::FromString(TEXT("총을 소지해야 사용할 수 있습니다")), FLinearColor::Red);
		}
		return; // 총 미장착 시 아이템 소모 안 함
	}

	ActiveWeapon->ActivateInfiniteAmmo(Duration, FireRateMultiplier);

	if (NCPC)
	{
		NCPC->Client_ShowNotification(FText::FromString(TEXT("무한 탄약이 활성화되었습니다")), FLinearColor::Yellow);
	}

	ConsumeItem(Interactor);
}