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

void UHitCheckNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    // 헌호수정 - 공격 시작 시 이전 히트 기록 초기화
    HitActors.Empty();
}

void UHitCheckNotify::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    DoHitCheck(MeshComp);
}

void UHitCheckNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    // 헌호수정 - 공격 끝나면 히트 기록 비우기
    HitActors.Empty();
}

void UHitCheckNotify::DoHitCheck(USkeletalMeshComponent* MeshComp)
{
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

    // 헌호수정 - 소켓 있으면 소켓 간 스피어트레이스 (도끼), 없으면 전방 스피어트레이스 (크로우바)
    if (WeaponData->TrailStartSocket != NAME_None && WeaponData->TrailEndSocket != NAME_None)
    {
        AActor* WeaponActor = Combat->GetSpawnedWeaponActor();
        if (WeaponActor)
        {
            UMeshComponent* WeaponMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
            if (!WeaponMesh)
                WeaponMesh = WeaponActor->FindComponentByClass<UStaticMeshComponent>();

            if (WeaponMesh)
            {
                const FVector TrailStart = WeaponMesh->GetSocketLocation(WeaponData->TrailStartSocket);
                const FVector TrailEnd   = WeaponMesh->GetSocketLocation(WeaponData->TrailEndSocket);
                const float Radius = WeaponData->HitSphereRadius;

                bHit = World->SweepMultiByChannel(
                    HitResults, TrailStart, TrailEnd, FQuat::Identity,
                    ECollisionChannel::ECC_Pawn,
                    FCollisionShape::MakeSphere(Radius),
                    Params
                );

                DrawDebugSphere(World, TrailStart, Radius, 12,
                    bHit ? FColor::Red : FColor::Green, false, 0.05f, 0, 1.f);
                DrawDebugSphere(World, TrailEnd, Radius, 12,
                    bHit ? FColor::Red : FColor::Green, false, 0.05f, 0, 1.f);
            }
        }
    }
    else
    {
        const FVector Start = OwnerChar->GetActorLocation();
        const FVector End   = Start + OwnerChar->GetActorForwardVector() * WeaponData->HitTraceRange;
        const float Radius = WeaponData->HitSphereRadius;

        bHit = World->SweepMultiByChannel(
            HitResults, Start, End, FQuat::Identity,
            ECollisionChannel::ECC_Pawn,
            FCollisionShape::MakeSphere(Radius),
            Params
        );

        DrawDebugSphere(World, End, Radius, 12,
            bHit ? FColor::Red : FColor::Green, false, 0.05f, 0, 2.f);
    }

    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerChar);
    if (!SourceASC) return;

    bool bHitMonster = false;
    FVector FirstHitLocation = FVector::ZeroVector;

    for (const FHitResult& Hit : HitResults)
    {
        AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(Hit.GetActor());
        if (!Monster) continue;

        // 헌호수정 - 이미 이번 공격에서 맞은 액터는 스킵 (중복 히트 방지)
        if (HitActors.Contains(Monster)) continue;
        HitActors.Add(Monster);

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

    if (bHitMonster && !WeaponData->HitSound.IsNull())
    {
        USoundBase* Sound = WeaponData->HitSound.LoadSynchronous();
        if (Sound)
            UGameplayStatics::PlaySoundAtLocation(World, Sound, FirstHitLocation);
    }

    // 헌호수정 - 적 히트 시 카메라 쉐이크 (로컬 플레이어에게만 적용)
    if (bHitMonster && HitShakeClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController()))
        {
            PC->ClientStartCameraShake(HitShakeClass);
        }
    }
}
