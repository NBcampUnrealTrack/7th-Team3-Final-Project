#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "NCCameraShakeBase.generated.h"

// 헌호수정 - 카메라 쉐이크 베이스 클래스 (BP에서 파생해서 값 튜닝)
UCLASS(Blueprintable)
class NAKWONCLONE_API UNCCameraShakeBase : public UCameraShakeBase
{
	GENERATED_BODY()
};
