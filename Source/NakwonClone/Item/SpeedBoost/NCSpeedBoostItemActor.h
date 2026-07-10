#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCSpeedBoostItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCSpeedBoostItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|SpeedBoost")
	float SpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|SpeedBoost")
	float Duration = 5.f;

	virtual void Interact_Implementation(AActor* Interactor) override;
};