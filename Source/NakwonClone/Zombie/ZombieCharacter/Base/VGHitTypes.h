#pragma once

#include "CoreMinimal.h"
#include "VGHitTypes.generated.h"

// 피격 부위
UENUM(BlueprintType)
enum class EVGHitBodyPart : uint8
{
    None  UMETA(DisplayName = "None"),
    Head  UMETA(DisplayName = "Head"),
    Upper UMETA(DisplayName = "Upper"),
    Lower UMETA(DisplayName = "Lower")
};

// 본 이름 → 부위 분류 (UE5 마네킹/메타휴먼 본 기준, 소문자 부분일치)
inline EVGHitBodyPart ClassifyBodyPart(FName BoneName)
{
    const FString Bone = BoneName.ToString().ToLower();

    if (Bone.Contains(TEXT("head")) || Bone.Contains(TEXT("neck")))
        return EVGHitBodyPart::Head;

    if (Bone.Contains(TEXT("thigh")) || Bone.Contains(TEXT("calf")) ||
        Bone.Contains(TEXT("foot")) || Bone.Contains(TEXT("ball")))
        return EVGHitBodyPart::Lower;

    if (Bone.Contains(TEXT("spine")) || Bone.Contains(TEXT("clavicle")) ||
        Bone.Contains(TEXT("arm")) || Bone.Contains(TEXT("hand")) ||
        Bone.Contains(TEXT("pelvis")) || Bone.Contains(TEXT("shoulder")))
        return EVGHitBodyPart::Upper;

    return EVGHitBodyPart::None;
}

// 부위별 데미지 배율 (여기 숫자만 바꾸면 됨)
inline float GetBodyPartDamageMultiplier(EVGHitBodyPart Part)
{
    switch (Part)
    {
    case EVGHitBodyPart::Head:  return 2.0f;
    case EVGHitBodyPart::Upper: return 1.0f;
    case EVGHitBodyPart::Lower: return 0.7f;
    default:                    return 1.0f; // None → 기본 배율 (분류 실패 시 안전)
    }
}