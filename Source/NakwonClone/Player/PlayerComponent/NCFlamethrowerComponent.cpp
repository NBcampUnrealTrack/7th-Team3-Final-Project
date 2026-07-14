#include "NCFlamethrowerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Common/NCGameplayTags.h"
#include "GAS/Effect/GE_Damage.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCEquipmentComponent.h"

UNCFlamethrowerComponent::UNCFlamethrowerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UNCFlamethrowerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DurationTimerHandle);
        World->GetTimerManager().ClearTimer(DamageTickTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void UNCFlamethrowerComponent::EquipFlamethrower(float Duration)
{
    if (bIsActive) return;

    ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwner());
    if (!Player) return;

    if (UNCEquipmentComponent* EquipComp = Player->GetEquipmentComponent())
    {
        EquipComp->HideActiveWeaponVisualOnly();
    }

    AttachFlamethrowerMesh();

    bIsActive = true;

    GetWorld()->GetTimerManager().SetTimer(
        DurationTimerHandle, this, &UNCFlamethrowerComponent::OnDurationExpired, Duration, false);
}

void UNCFlamethrowerComponent::UnequipFlamethrower()
{
    if (!bIsActive) return;

    StopFire();
    DetachFlamethrowerMesh();

    if (ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwner()))
    {
        if (UNCEquipmentComponent* EquipComp = Player->GetEquipmentComponent())
        {
            EquipComp->ShowActiveWeaponVisualAgain();
        }
    }

    bIsActive = false;

    GetWorld()->GetTimerManager().ClearTimer(DurationTimerHandle);
}

void UNCFlamethrowerComponent::OnDurationExpired()
{
    UnequipFlamethrower();
}

void UNCFlamethrowerComponent::AttachFlamethrowerMesh()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return;

    if (USkeletalMesh* SkelMesh = FlamethrowerSkeletalMesh.LoadSynchronous())
    {
        USkeletalMeshComponent* SkelComp = NewObject<USkeletalMeshComponent>(OwnerChar);
        SkelComp->SetSkeletalMesh(SkelMesh);
        SkelComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SkelComp->RegisterComponent();
        SkelComp->AttachToComponent(
            OwnerChar->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            HandSocketName);
        EquippedMeshComp = SkelComp;
    }
    else if (UStaticMesh* StatMesh = FlamethrowerStaticMesh.LoadSynchronous())
    {
        UStaticMeshComponent* StatComp = NewObject<UStaticMeshComponent>(OwnerChar);
        StatComp->SetStaticMesh(StatMesh);
        StatComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StatComp->RegisterComponent();
        StatComp->AttachToComponent(
            OwnerChar->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            HandSocketName);
        EquippedMeshComp = StatComp;
    }
}

void UNCFlamethrowerComponent::DetachFlamethrowerMesh()
{
    if (EquippedMeshComp)
    {
        EquippedMeshComp->DestroyComponent();
        EquippedMeshComp = nullptr;
    }
}

void UNCFlamethrowerComponent::StartFire()
{
    if (!bIsActive || bIsFiring) return;

    bIsFiring = true;

    GetWorld()->GetTimerManager().SetTimer(
        DamageTickTimerHandle, this, &UNCFlamethrowerComponent::ApplyConeDamageTick, TickInterval, true, 0.f);

    if (EquippedMeshComp && !FlameVFX.IsNull())
    {
        if (UNiagaraSystem* VFX = FlameVFX.LoadSynchronous())
        {
            FlameVFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
                VFX, EquippedMeshComp, MuzzleSocketName,
                FVector::ZeroVector, FRotator::ZeroRotator,
                EAttachLocation::SnapToTarget, true);
        }
    }

    if (EquippedMeshComp && !FireLoopSound.IsNull())
    {
        if (USoundBase* Sound = FireLoopSound.LoadSynchronous())
        {
            FireLoopAudioComp = UGameplayStatics::SpawnSoundAttached(
                Sound, EquippedMeshComp, MuzzleSocketName);
        }
    }
}

void UNCFlamethrowerComponent::StopFire()
{
    if (!bIsFiring) return;

    bIsFiring = false;

    GetWorld()->GetTimerManager().ClearTimer(DamageTickTimerHandle);

    if (FlameVFXComp)
    {
        FlameVFXComp->DestroyComponent();
        FlameVFXComp = nullptr;
    }

    if (FireLoopAudioComp)
    {
        FireLoopAudioComp->Stop();
        FireLoopAudioComp->DestroyComponent();
        FireLoopAudioComp = nullptr;
    }
}

void UNCFlamethrowerComponent::ApplyConeDamageTick()
{
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar || !EquippedMeshComp) return;

    const FVector Origin = EquippedMeshComp->DoesSocketExist(MuzzleSocketName)
        ? EquippedMeshComp->GetSocketLocation(MuzzleSocketName)
        : EquippedMeshComp->GetComponentLocation();
    const FVector Forward = OwnerChar->GetActorForwardVector();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerChar);

    GetWorld()->OverlapMultiByObjectType(
        Overlaps, Origin, FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(ConeRange), Params);

    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerChar);
    if (!SourceASC) return;

    const float HalfAngleCos = FMath::Cos(FMath::DegreesToRadians(ConeAngle * 0.5f));

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor || OtherActor == OwnerChar || !OtherActor->IsA<ACharacter>()) continue;

        const FVector ToTarget = (OtherActor->GetActorLocation() - Origin).GetSafeNormal();
        if (FVector::DotProduct(Forward, ToTarget) < HalfAngleCos) continue;

        UAbilitySystemComponent* TargetASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
        if (!TargetASC) continue;

        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(UGE_Damage::StaticClass(), 1.f, Context);
        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(NCData::Damage, -DamagePerTick);
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
        }
    }
}
