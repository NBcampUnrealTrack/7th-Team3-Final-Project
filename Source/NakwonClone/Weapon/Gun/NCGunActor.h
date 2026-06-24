#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"

#include "NCGunActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCGunActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCGunActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun")
	FName GunID;

	virtual void Interact_Implementation(AActor* Interactor) override;
};
