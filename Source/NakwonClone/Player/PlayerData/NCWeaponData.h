#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NCWeaponData.generated.h"

//DataTable 행 구조 - 무기 타입 정보 (공유, 1개만 존재)
USTRUCT(BlueprintType)
struct FNCWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	//무기 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FName WeaponName;

	//무기 타입 태그 (Weapon.Type.OneHanded 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag WeaponTypeTag;

	//무기 무게 태그 (Weapon.Weight.Medium 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
	FGameplayTag WeightTag;

	//전투 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Damage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxDurability = 100.f;

	//공격 속도 (몽타주 재생 속도에 적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackSpeed = 1.0f;

	//무기 교체 속도 (장착/해제 몽타주 속도에 적용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float SwapSpeed = 1.0f;

	//히트박스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FVector HitBoxExtent = FVector(50.f, 30.f, 10.f);

	//애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> BrokenMontage;
};

//개별 무기 인스턴스 - 줍는 순간 생성, 내구도 개별 관리
USTRUCT(BlueprintType)
struct FNCWeaponInstance
{
	GENERATED_BODY()

	//고유 ID (줍는 순간 발급, 같은 종류여도 구별 가능)
	UPROPERTY(BlueprintReadOnly)
	FGuid UniqueID;

	//DataTable 조회 키 (예: "Crowbar")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponID;

	//현재 내구도 (개별 관리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDurability = 100.f;

	//파손 여부
	UPROPERTY(BlueprintReadOnly)
	bool bIsBroken = false;

	//유효한 인스턴스인지 확인
	bool IsValid() const { return UniqueID.IsValid() && !WeaponID.IsNone(); }
};