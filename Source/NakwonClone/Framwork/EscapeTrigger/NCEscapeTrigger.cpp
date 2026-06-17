#include "NCEscapeTrigger.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "NakwonClone/Framwork/Gamemode/NCGameMode.h"

ANCEscapeTrigger::ANCEscapeTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this, &ANCEscapeTrigger::OnTriggerBeginOverlap);
}

void ANCEscapeTrigger::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 서버에서만 처리
	if (!HasAuthority())
	{
		return;
	}

	// 플레이어 확인
	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// 인벤토리 저장
	if (UNCPlayerInventoryComponent* Inventory = Player->GetInventoryComponent())
	{
		Inventory->SaveInventoryData();
	}

	// 게임 클리어 처리
	if (ANCGameMode* GameMode = GetWorld()->GetAuthGameMode<ANCGameMode>())
	{
		GameMode->HandleMatchEnd(true);
	}

	// 클리어 UI 표시
	if (ClearWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			UUserWidget* ClearWidget = CreateWidget<UUserWidget>(PC, ClearWidgetClass);
			if (ClearWidget)
			{
				ClearWidget->AddToViewport();
				PC->SetShowMouseCursor(true);
				PC->SetInputMode(FInputModeUIOnly());
			}
		}
	}
}