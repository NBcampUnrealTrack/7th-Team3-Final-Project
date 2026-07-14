#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCFlamethrowerItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCFlamethrowerItemActor : public ANCItemActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Flamethrower")
    float Duration = 15.f;

    virtual void Interact_Implementation(AActor* Interactor) override;
};
