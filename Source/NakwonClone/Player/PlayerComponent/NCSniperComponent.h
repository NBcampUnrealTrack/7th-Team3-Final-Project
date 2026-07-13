#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerComponent/NCGunComponent.h"
#include "NCSniperComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCSniperComponent : public UNCGunComponent
{
    GENERATED_BODY()

public:
    UNCSniperComponent();
};