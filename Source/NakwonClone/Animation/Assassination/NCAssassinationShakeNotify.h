#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Camera/CameraShakeBase.h"
#include "NCAssassinationShakeNotify.generated.h"

UCLASS()
class NAKWONCLONE_API UNCAssassinationShakeNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    // 헌호수정 - 암살 타격 시 카메라 쉐이크 클래스 (에디터에서 할당)
    UPROPERTY(EditAnywhere, Category = "CameraShake")
    TSubclassOf<UCameraShakeBase> ShakeClass;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
};
