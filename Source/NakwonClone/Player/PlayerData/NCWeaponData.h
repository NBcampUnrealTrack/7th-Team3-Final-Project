#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "NCWeaponData.generated.h"

//H
class UBlendSpace;
class USoundBase;
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

	//스폰할 무기 액터 클래스 (BP_Crowbar 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor")
	TSoftClassPtr<AActor> WeaponActorClass;

	//장착할 캐릭터 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FName AttachSocketName = NAME_None;

	//양손 IK 사용 여부 (양손무기)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	bool bUseTwoHandIK = false;

	//왼손 IK 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FName LeftHandIKSocketName = NAME_None;

	//오른손 IK 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FName RightHandIKSocketName = NAME_None;

	//왼손 IK 사용 여부 (단일 왼손 IK, 기존 호환용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	bool bUseLeftHandIK = false;

	// 소켓 기반 스피어 트레이스 히트 판정 (양손무기용, 비어있으면 전방 스피어 트레이스 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	FName TrailStartSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	FName TrailEndSocket = NAME_None;

	// 헌호수정 - 스피어트레이스 반경 (소켓 방식/전방 방식 공통)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	float HitSphereRadius = 15.f;

	// 헌호수정 - 전방 스피어트레이스 사거리 (소켓 없을 때 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTrace")
	float HitTraceRange = 150.f;

	// 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TSoftObjectPtr<USoundBase> SwingSound;

	//헌호수정 - 사용할 콤보 섹션 목록 (비어있으면 Attack1만 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<FName> AttackSections;

	//애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> HeavyAttackMontage;

	//H AttackMontage가 풀바디인지 (true=하체까지 덮음, false=상체만)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bAttackFullBody = false;

	//H HeavyAttackMontage가 풀바디인지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bHeavyAttackFullBody = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> BrokenMontage;

	//H 이동 BlendSpace - 서 있을 때
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UBlendSpace> LocomotionBS_Standing;

	//H 이동 BlendSpace - 앉아 있을 때
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UBlendSpace> LocomotionBS_Crouching;
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