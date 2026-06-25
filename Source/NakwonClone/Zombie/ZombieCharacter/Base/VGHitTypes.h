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
    Arm   UMETA(DisplayName = "Arm"),
    Lower UMETA(DisplayName = "Lower")
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
};

inline EVGHitBodyPart ClassifyBodyPart(FName BoneName)
{
    if (BoneName.IsNone())
        return EVGHitBodyPart::None;

    const FString Bone = BoneName.ToString().ToLower();

    // 머리 / 목
    if (Bone.Contains(TEXT("head")) || Bone.Contains(TEXT("neck")))
        return EVGHitBodyPart::Head;

    // 다리 / 발
    if (Bone.Contains(TEXT("thigh")) || Bone.Contains(TEXT("calf")) ||
        Bone.Contains(TEXT("foot")) || Bone.Contains(TEXT("ball")) ||
        Bone.Contains(TEXT("toe")))
        return EVGHitBodyPart::Lower;

    // 팔 (어깨~손, 손가락 포함) — clavicle은 몸통으로 두려고 제외
    if (Bone.Contains(TEXT("upperarm")) || Bone.Contains(TEXT("lowerarm")) ||
        Bone.Contains(TEXT("hand")) || Bone.Contains(TEXT("finger")) ||
        Bone.Contains(TEXT("index")) || Bone.Contains(TEXT("middle")) ||
        Bone.Contains(TEXT("ring")) || Bone.Contains(TEXT("pinky")) ||
        Bone.Contains(TEXT("thumb")))
        return EVGHitBodyPart::Arm;

    // 그 외(spine, pelvis, clavicle 등)는 몸통
    return EVGHitBodyPart::Body;
}

inline float GetBodyPartDamageMultiplier(EVGHitBodyPart Part)
{
    switch (Part)
    {
    case EVGHitBodyPart::Head:  return 2.0f;
    case EVGHitBodyPart::Body:  return 1.0f;
    case EVGHitBodyPart::Arm:   return 0.8f;
    case EVGHitBodyPart::Lower: return 0.7f;
    default:                    return 1.0f; // None → 기본 (분류 실패 시 안전)
    }
}