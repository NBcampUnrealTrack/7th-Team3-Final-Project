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

	// 드롭 시 탄약 보존용 (-1 = 새 총, 0 이상 = 드롭된 총의 잔탄)
	UPROPERTY(BlueprintReadOnly, Category = "Gun")
	int32 SavedCurrentAmmo = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Gun")
	int32 SavedReserveAmmo = -1;

	virtual void Interact_Implementation(AActor* Interactor) override;
};
