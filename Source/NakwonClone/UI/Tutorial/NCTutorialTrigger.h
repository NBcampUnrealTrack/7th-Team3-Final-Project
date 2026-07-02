#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NCTutorialTrigger.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class NAKWONCLONE_API ANCTutorialTrigger : public AActor
{
    GENERATED_BODY()

public:
    ANCTutorialTrigger();

    UPROPERTY(EditAnywhere, Category = "Tutorial")
    TArray<FName> StepIDs;

    UPROPERTY(EditAnywhere, Category = "Tutorial")
    TSubclassOf<UUserWidget> KeyGuideWidgetClass;

    UPROPERTY(EditAnywhere, Category = "Tutorial")
    bool bTriggerOnce = true;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Tutorial")
    TObjectPtr<UBoxComponent> TriggerBox;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    bool bHasTriggered = false;
};
