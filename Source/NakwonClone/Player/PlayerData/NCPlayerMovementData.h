#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NCPlayerMovementData.generated.h"

USTRUCT(BlueprintType)
struct FNcPlayerMovementData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MaxAcceleration;
	
	FNcPlayerMovementData()
		: MovementSpeed(500.f)
		, MaxAcceleration(1500.f)
	{}
};
