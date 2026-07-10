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
	// 이 총알이 채워줄 슬롯 (Shotgun / Rifle / Sidearm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Ammo")
	ENCGunSlot AmmoSlotType = ENCGunSlot::Rifle;

	// 습득 시 채워지는 탄약 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Ammo")
	int32 AmmoAmount = 30;

	virtual void Interact_Implementation(AActor* Interactor) override;
};