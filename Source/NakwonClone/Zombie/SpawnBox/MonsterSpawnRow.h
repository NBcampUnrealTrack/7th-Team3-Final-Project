#pragma once

#include "CoreMinimal.h"
#include "MonsterSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct NAKWONCLONE_API FMonsterSpawnRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> MonsterClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance;
};
