#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NCEscapeTrigger.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class NAKWONCLONE_API ANCEscapeTrigger : public AActor
{
	GENERATED_BODY()

public:
	ANCEscapeTrigger();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Escape")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Escape")
	TSubclassOf<UUserWidget> ClearWidgetClass;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};