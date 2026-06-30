#include "NCAssassinationShakeNotify.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

void UNCAssassinationShakeNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!ShakeClass) return;

    ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    // 헌호수정 - 로컬 플레이어에게만 쉐이크 적용
    APlayerController* PC = Cast<APlayerController>(Owner->GetController());
    if (PC && Owner->IsLocallyControlled())
        PC->ClientStartCameraShake(ShakeClass);
}
