#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FMonsterTypeRow.generated.h"

class USkeletalMesh;
class UAnimInstance;

USTRUCT(BlueprintType)
struct FMonsterTypeRow : public FTableRowBase
{
	GENERATED_BODY()
public:
	// 외형
	UPROPERTY(EditAnywhere) TSoftObjectPtr<USkeletalMesh> Mesh;
	// 상태별 애니를 담당할 AnimBP (타입마다 다르게)
	UPROPERTY(EditAnywhere) TSubclassOf<UAnimInstance> AnimClass;
	// 스탯
	UPROPERTY(EditAnywhere) float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere) float Damage = 10.f;
	UPROPERTY(EditAnywhere) float MoveSpeed = 200.f;
};