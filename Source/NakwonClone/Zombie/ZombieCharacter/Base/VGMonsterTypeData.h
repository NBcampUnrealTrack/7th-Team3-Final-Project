#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGHitTypes.h" 
#include "VGMonsterTypeData.generated.h"

class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;
class UGameplayEffect;
class USoundBase;

UENUM(BlueprintType)
enum class EVGMonsterType : uint8
{
    Walker    UMETA(DisplayName = "Walker"),   // 기본 워커 (완성)
    Witch     UMETA(DisplayName = "Witch"),    // 자극→큰소리→강공격
    Tank      UMETA(DisplayName = "Tank"),     // 맷집(HP 큼)
    Runner    UMETA(DisplayName = "Runner"),   // 이동 속도 빠름
    MAX       UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FVGMonsterTypeRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Appearance")
    TObjectPtr<USkeletalMesh> Mesh = nullptr;

    UPROPERTY(EditAnywhere, Category = "Appearance")
    TSubclassOf<UAnimInstance> AnimClass = nullptr;

    UPROPERTY(EditAnywhere, Category = "Appearance")
    float MeshScale = 1.f;

    UPROPERTY(EditAnywhere, Category = "Appearance")
    TObjectPtr<UBlendSpace> LocomotionBS = nullptr;

    UPROPERTY(EditAnywhere, Category = "Animation")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages;

    UPROPERTY(EditAnywhere, Category = "Animation")
    TObjectPtr<UAnimMontage> SpecialMontage = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<UGameplayEffect> AttackEffectClass = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Combat")
    float HitReactChance = 0.7f;

    UPROPERTY(EditAnywhere, Category = "Stat")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, Category = "Stat")
    float MoveSpeed = 150.f;
    
    UPROPERTY(EditAnywhere, Category = "Stat")
    float PatrolSpeed = 100.f;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> HitSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TMap<EVGHitBodyPart, TObjectPtr<USoundBase>> HitSoundsByPart;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> IdleSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Sound")
    float IdleSoundCooldown = 5.f;   // 재생 종료 후 쉬는 시간
};