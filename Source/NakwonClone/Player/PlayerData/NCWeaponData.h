#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NCWeaponData.generated.h"

USTRUCT(BlueprintType)
struct FNCMeleeWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName;

	//GAS 전환 시 - GameplayEffect로 대체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector HitBoxExtent = FVector(50.f, 30.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSoftClassPtr<AActor> WeaponClass;
};