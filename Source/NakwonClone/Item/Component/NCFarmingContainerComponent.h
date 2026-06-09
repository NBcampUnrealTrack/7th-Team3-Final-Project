#pragma once

#include "CoreMinimal.h"
#include "Inventory/NCInventoryBaseComponent.h"

#include "NCFarmingContainerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCFarmingContainerComponent : public UNCInventoryBaseComponent
{
	GENERATED_BODY()

public:
	UNCFarmingContainerComponent();
	
};