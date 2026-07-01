#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NCWeaponData.generated.h"

class USoundBase;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FNCWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag WeaponTypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag WeightTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxDurability = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float SwapSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FVector HitBoxExtent = FVector(50.f, 30.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor")
	TSoftClassPtr<AActor> WeaponActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FName AttachSocketName = NAME_None;

	// 양손 IK: 켜면 왼손+오른손 모두 각 소켓으로 IK 적용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	bool bUseTwoHandIK = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FName LeftHandIKSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FName RightHandIKSocketName = NAME_None;

	// 왼손 전용 IK: bUseTwoHandIK가 꺼져 있어도 왼손만 IK 적용 (오른손 미적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	bool bUseLeftHandIK = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	FName TrailStartSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	FName TrailEndSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	float HitSphereRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	float HitTraceRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> SwingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Melee")
	TArray<FName> AttackSections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Melee")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Melee")
	TSoftObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Melee")
	bool bAttackFullBody = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Melee")
	bool bHeavyAttackFullBody = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Common")
	TSoftObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Common")
	TSoftObjectPtr<UAnimMontage> UnequipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gun")
	TSoftObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gun")
	TSoftObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gun")
	TSoftObjectPtr<UAnimMontage> EmptyReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Gun")
	TSoftObjectPtr<UAnimMontage> PumpMontage;

	// 헌호수정 - 암살
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assassination")
	TSoftObjectPtr<UAnimMontage> AssassinationMontage;     // 플레이어 휘두르기 모션

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assassination")
	TSoftObjectPtr<UAnimMontage> AssassinationVictimMontage; // 좀비 당하는 모션

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assassination")
	float AssassinationKillTime = 0.8f;                    // 즉사까지 걸리는 시간
};

USTRUCT(BlueprintType)
struct FNCWeaponInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid UniqueID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDurability = 100.f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsBroken = false;

	bool IsValid() const
	{
		return UniqueID.IsValid() && !WeaponID.IsNone();
	}
};