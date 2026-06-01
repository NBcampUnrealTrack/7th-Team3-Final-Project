#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NCInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetChangedSignature, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void Interact();
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractTargetChangedSignature OnInteractTargetChanged;
	
private:
	FTimerHandle TimerHandle_UpdateInteractable;
	
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Settings")
	float InteractionCheckInterval = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Settings")
	float InteractionSearchRadius = 200.0f;
	
	UPROPERTY()
	AActor* CurrentInteractableTarget = nullptr;
	
	void UpdateInteractableTarget();
	
	void SetHighlight(AActor* TargetActor, bool bHighlight);
};
