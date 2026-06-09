#pragma once

#include "CoreMinimal.h"
#include "NCInventoryBaseComponent.h"

#include "NCStashInventoryComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCStashInventoryComponent : public UNCInventoryBaseComponent
{
	GENERATED_BODY()

public:
	UNCStashInventoryComponent();
};
