#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
#include "NCAmmoCrateActor.generated.h"

class UStaticMeshComponent;
class USoundBase;
class UAnimMontage;

UCLASS()
class NAKWONCLONE_API ANCAmmoCrateActor : public AActor, public INCInteractableInterface
{
	GENERATED_BODY()

public:
	ANCAmmoCrateActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoCrate")
	TObjectPtr<UStaticMeshComponent> CrateMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AmmoCrate")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoCrate|Sound")
	TObjectPtr<USoundBase> RefillSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoCrate|Animation")
	TObjectPtr<UAnimMontage> RefillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AmmoCrate|Highlight")
	int32 HighlightStencilValue = 1;

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void ToggleHighlight_Implementation(bool bHighlight) override;
	virtual FText GetInteractPrompt_Implementation() override;
};
