#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "WeaponTrailNotify.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class NAKWONCLONE_API UWeaponTrailNotify : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    // 트레일 Niagara (에디터에서 NS_SwordTrail 지정)
    UPROPERTY(EditAnywhere, Category = "Trail")
    TObjectPtr<UNiagaraSystem> TrailSystem;

    // 붙일 소켓 (비우면 무기 데이터의 TrailEndSocket 사용)
    UPROPERTY(EditAnywhere, Category = "Trail")
    FName SocketOverride = NAME_None;

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

private:
    // 스폰된 트레일 핸들 (테스트용)
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> TrailNiagara;
};
