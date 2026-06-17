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

	// 헌호수정 - 감속도 (클수록 빨리 멈춤, 작을수록 미끄러짐)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BrakingDeceleration;

	// 헌호수정 - 마찰력 배율 (1.0 = 자연스러움, 2.0 = 딱딱하게 멈춤)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float BrakingFrictionFactor;

	// 헌호수정 - 방향 전환 속도 (낮을수록 몸이 천천히 돌아감)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RotationRate;

	FNcPlayerMovementData()
		: MovementSpeed(500.f)
		, MaxAcceleration(1500.f)
		, BrakingDeceleration(800.f)
		, BrakingFrictionFactor(1.0f)
		, RotationRate(270.f)
	{}
};
