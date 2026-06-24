#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NCGunComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCGunComponent();

};
