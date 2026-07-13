#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "Weapon/Gun/GunType/NCGunType.h"
#include "NCAmmoItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCAmmoItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Ammo")
	FName AmmoType = NAME_None;

	// 습득 시 채워지는 탄약 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Ammo")
	int32 AmmoAmount = 30;

	virtual void Interact_Implementation(AActor* Interactor) override;
};