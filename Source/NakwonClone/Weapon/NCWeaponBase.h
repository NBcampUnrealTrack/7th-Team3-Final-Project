#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NCWeaponBase.generated.h"

UCLASS()
class NAKWONCLONE_API ANCWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ANCWeaponBase();

	void AttachToCharacter(USkeletalMeshComponent* CharacterMesh, FName SocketName);
	void DetachFromCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponName;
};