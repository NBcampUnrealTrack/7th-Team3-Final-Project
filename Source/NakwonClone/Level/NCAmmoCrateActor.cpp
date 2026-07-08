#include "NCAmmoCrateActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCEquipmentComponent.h"

ANCAmmoCrateActor::ANCAmmoCrateActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CrateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
	RootComponent = CrateMesh;

	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(CrateMesh);
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidget->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	InteractionWidget->SetVisibility(false);
}

void ANCAmmoCrateActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(Interactor);
	if (!PlayerCharacter) return;

	UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent();
	if (!EquipComp) return;

	EquipComp->RefillAllReserveAmmo();

	if (RefillSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RefillSound, GetActorLocation());
	}

	if (RefillMontage)
	{
		PlayerCharacter->PlayAnimMontage(RefillMontage);
	}
}

bool ANCAmmoCrateActor::CanInteract_Implementation(AActor* Interactor)
{
	return true;
}

void ANCAmmoCrateActor::ToggleHighlight_Implementation(bool bHighlight)
{
	static const FName NoHighlightTag(TEXT("NoHighlight"));

	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);

	for (UStaticMeshComponent* MeshComp : MeshComponents)
	{
		if (!MeshComp || MeshComp->ComponentHasTag(NoHighlightTag)) continue;

		MeshComp->SetRenderCustomDepth(bHighlight);
		MeshComp->SetCustomDepthStencilValue(HighlightStencilValue);
	}

	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(bHighlight);
	}
}

FText ANCAmmoCrateActor::GetInteractPrompt_Implementation()
{
	return FText::FromString(TEXT("탄약 보급"));
}
