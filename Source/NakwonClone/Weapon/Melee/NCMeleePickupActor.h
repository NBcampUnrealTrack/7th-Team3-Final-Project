#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCMeleePickupActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCMeleePickupActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCMeleePickupActor();

	// DT_WeaponData 행 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee")
	FName WeaponID;

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
};
