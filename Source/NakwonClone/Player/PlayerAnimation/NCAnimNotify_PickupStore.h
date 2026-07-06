#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NCAnimNotify_PickupStore.generated.h"

UCLASS()
class NAKWONCLONE_API UNCAnimNotify_PickupStore : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};