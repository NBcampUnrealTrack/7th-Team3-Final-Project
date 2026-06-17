#include "HitCheckNotify.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NakwonClone/GAS/Effect/GE_Damage.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UHitCheckNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    ACharacter* OwnerChar = Cast<ACharacter>(MeshComp->GetOwner());
    if (!OwnerChar) return;

    if (!OwnerChar->HasAuthority()) return;

    UWorld* World = OwnerChar->GetWorld();
    if (!World) return;

    UNCCombatComponent* Combat = OwnerChar->FindComponentByClass<UNCCombatComponent>();
    if (!Combat) return;

    FNCWeaponData* WeaponData = Combat->GetEquippedWeaponData();
    if (!WeaponData) return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerChar);

    TArray<FHitResult> HitResults;
    bool bHit = false;

    // 헌호수정 - 소켓 있으면 소켓 간 스피어트레이스, 없으면 전방 스피어트레이스 (둘 다 스피어)
    if (WeaponData->TrailStartSocket != NAME_None && WeaponData->TrailEndSocket != NAME_None)
    {
        // 헌호수정 - 도끼 등 양손무기 - 소켓 간 라인트레이스
        AActor* WeaponActor = Combat->GetSpawnedWeaponActor();
        if (WeaponActor)
        {
            UMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
            if (!WeaponMesh)
                WeaponMesh = WeaponActor->FindComponentByClass<UStaticMeshComponent>();

            if (WeaponMesh)
            {
                const FVector TrailStart = WeaponMesh->GetSocketLocation(WeaponData->TrailStartSocket);
                const FVector TrailEnd = WeaponMesh->GetSocketLocation(WeaponData->TrailEndSocket);

                bHit = World->LineTraceMultiByChannel(
                    HitResults, TrailStart, TrailEnd,
                    ECollisionChannel::ECC_Pawn,
                    Params
                );

                DrawDebugLine(World, TrailStart, TrailEnd,
                    bHit ? FColor::Red : FColor::Green, false, 3.f, 0, 2.f);
            }
        }
    }
    else
    {
        // 크로우바 등 한손무기 - 전방 스피어트레이스
        const FVector Start = OwnerChar->GetActorLocation();
        const FVector End = Start + OwnerChar->GetActorForwardVector() * WeaponData->HitTraceRange;
        const float Radius = WeaponData->HitSphereRadius;

        bHit = World->SweepMultiByChannel(
            HitResults, Start, End, FQuat::Identity,
            ECollisionChannel::ECC_Pawn,
            FCollisionShape::MakeSphere(Radius),
            Params
        );

        DrawDebugSphere(World, End, Radius, 12,
            bHit ? FColor::Red : FColor::Green, false, 3.f, 0, 2.f);
    }

    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerChar);
    if (!SourceASC) return;

    bool bHitMonster = false;
    FVector FirstHitLocation = FVector::ZeroVector;

    for (const FHitResult& Hit : HitResults)
    {
        AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(Hit.GetActor());
        if (!Monster) continue;

        UAbilitySystemComponent* TargetASC = Monster->GetAbilitySystemComponent();
        if (!TargetASC) continue;

        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(
            UGE_Damage::StaticClass(), 1.f, Context);

        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(NCData::Damage, -WeaponData->Damage);
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

            UE_LOG(LogTemp, Warning, TEXT("[HitCheckNotify] 좀비 데미지 적용: %.1f → %s"),
                WeaponData->Damage, *Monster->GetName());

            if (!bHitMonster)
                FirstHitLocation = Hit.ImpactPoint;

            bHitMonster = true;
        }
    }

    // 좀비 맞았을 때 히트 사운드 재생
    if (bHitMonster && !WeaponData->HitSound.IsNull())
    {
        USoundBase* Sound = WeaponData->HitSound.LoadSynchronous();
        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(World, Sound, FirstHitLocation);
        }
    }
}
