#include "WeaponTrailNotify.h"
#include "GameFramework/Character.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

void UWeaponTrailNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!TrailSystem || !MeshComp) return;

    ACharacter* OwnerChar = Cast<ACharacter>(MeshComp->GetOwner());
    if (!OwnerChar) return;

    UNCCombatComponent* Combat = OwnerChar->FindComponentByClass<UNCCombatComponent>();
    if (!Combat) return;

    AActor* WeaponActor = Combat->GetSpawnedWeaponActor();
    if (!WeaponActor) return;

    UMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!WeaponMesh)
        WeaponMesh = WeaponActor->FindComponentByClass<UStaticMeshComponent>();
    if (!WeaponMesh) return;

    // 소켓: 오버라이드 있으면 그걸, 없으면 무기 데이터의 TrailEndSocket
    FName TipSocket = SocketOverride;
    if (TipSocket.IsNone())
    {
        if (FNCWeaponData* WeaponData = Combat->GetEquippedWeaponData())
            TipSocket = WeaponData->TrailEndSocket;
    }

    TrailNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
        TrailSystem, WeaponMesh, TipSocket,
        FVector::ZeroVector, FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget, true);
}

void UWeaponTrailNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    // 트레일 끄기 (남은 띠는 자연스럽게 사라짐)
    if (TrailNiagara)
    {
        TrailNiagara->Deactivate();
        TrailNiagara = nullptr;
    }
}
