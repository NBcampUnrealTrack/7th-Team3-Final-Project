#pragma once

#include "CoreMinimal.h"
#include "VGHitTypes.generated.h"

class UAnimMontage;

// 피격 부위
UENUM(BlueprintType)
enum class EVGHitBodyPart : uint8
{
    None  UMETA(DisplayName = "None"),
    Head  UMETA(DisplayName = "Head"),
    Body  UMETA(DisplayName = "Body"),
    ArmL  UMETA(DisplayName = "Arm (Left)"),
    ArmR  UMETA(DisplayName = "Arm (Right)"),
    LegL  UMETA(DisplayName = "Leg (Left)"),
    LegR  UMETA(DisplayName = "Leg (Right)")
};

USTRUCT(BlueprintType)
struct FVGHitMontageList
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Monster|Animation")
    TArray<TObjectPtr<UAnimMontage>> Montages;
};

// 피격 1회분 정보 (델리게이트로 전달)
USTRUCT(BlueprintType)
struct FVGHitData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Hit")
    EVGHitBodyPart BodyPart = EVGHitBodyPart::None;

    UPROPERTY(BlueprintReadWrite, Category = "Hit")
    FVector HitLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Hit")
    FVector HitNormal = FVector::ZeroVector;
};

inline EVGHitBodyPart ClassifyBodyPart(FName BoneName)
{
    if (BoneName.IsNone())
        return EVGHitBodyPart::None;

    const FString Bone = BoneName.ToString().ToLower();

    // 머리 / 목 (좌우 없음)
    if (Bone.Contains(TEXT("head")) || Bone.Contains(TEXT("neck")))
        return EVGHitBodyPart::Head;

    // 좌우 판별 (UE 본은 _l / _r 접미사로 끝남)
    const bool bRight = Bone.EndsWith(TEXT("_r"));

    // 다리 / 발
    if (Bone.Contains(TEXT("pelvis")) || Bone.Contains(TEXT("thigh")) ||
        Bone.Contains(TEXT("calf")) || Bone.Contains(TEXT("foot")) ||
        Bone.Contains(TEXT("ball")) || Bone.Contains(TEXT("toe")))
    {
        return bRight ? EVGHitBodyPart::LegR : EVGHitBodyPart::LegL;
    }

    // 팔 (어깨~손, 손가락 포함)
    if (Bone.Contains(TEXT("clavicle")) || Bone.Contains(TEXT("upperarm")) ||
        Bone.Contains(TEXT("lowerarm")) || Bone.Contains(TEXT("hand")) ||
        Bone.Contains(TEXT("index")) || Bone.Contains(TEXT("middle")) ||
        Bone.Contains(TEXT("ring")) || Bone.Contains(TEXT("pinky")) ||
        Bone.Contains(TEXT("thumb")))
    {
        return bRight ? EVGHitBodyPart::ArmR : EVGHitBodyPart::ArmL;
    }

    // 그 외(spine, pelvis, clavicle 등)는 몸통
    return EVGHitBodyPart::Body;
}

inline float GetBodyPartDamageMultiplier(EVGHitBodyPart Part)
{
    switch (Part)
    {
    case EVGHitBodyPart::Head:  return 2.0f;
    case EVGHitBodyPart::Body:  return 1.0f;
    case EVGHitBodyPart::ArmL:                       // 좌우 같은 배율
    case EVGHitBodyPart::ArmR:  return 0.8f;
    case EVGHitBodyPart::LegL:
    case EVGHitBodyPart::LegR:  return 0.7f;
    default:                    return 1.0f;         // None → 기본
    }
}