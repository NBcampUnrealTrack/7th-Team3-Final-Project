#pragma once

#include "CoreMinimal.h"
#include "VGMonsterTypes.generated.h"

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	Walker		UMETA(DisplayName = "Walker"),
	Runner		UMETA(DisplayName = "Runner"),
	Witch		UMETA(DisplayName = "Witch"),
};