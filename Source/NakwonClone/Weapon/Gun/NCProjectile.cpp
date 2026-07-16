#include "NCProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Common/NCGameplayTags.h"
#include "GAS/Effect/GE_Damage.h"
#include "GameplayEffectTypes.h" // FGameplayCueParameters
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "Item/NCItemActor.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h" //헌호수정 - 바렛 스나이퍼 킬

ANCProjectile::ANCProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_GameTraceChannel1);  // Projectile 채널
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore); // 발사체끼리 무시
    CollisionComp->bReturnMaterialOnMove = true; // Hit.PhysMaterial 채워서 표면별 이펙트 분기에 사용
    CollisionComp->OnComponentHit.AddDynamic(this, &ANCProjectile::OnHit);
    RootComponent = CollisionComp;

    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    MovementComp->UpdatedComponent  = CollisionComp;
    MovementComp->InitialSpeed       = 10000.f;
    MovementComp->MaxSpeed           = 10000.f;
    MovementComp->bRotationFollowsVelocity = true;
    MovementComp->bShouldBounce      = false;
    MovementComp->bSweepCollision    = true;
    MovementComp->ProjectileGravityScale = 0.f; // 낙차 없는 직선탄

    InitialLifeSpan = 3.f;
}

void ANCProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* OwnerActor = GetOwner()) CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);

    if (bTracePenetration)
    {
        ResolveTracePenetration(MaxRange);
        CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    else if (CheckPointBlankOverlap())
    {
        return;
    }

    // ProjectileSpeed는 GunComponent가 SpawnActor 직후 설정
    MovementComp->InitialSpeed = ProjectileSpeed;
    MovementComp->MaxSpeed     = ProjectileSpeed;
    if (ProjectileSpeed > 0.f)
        SetLifeSpan(MaxRange / ProjectileSpeed);

    // 총알 궤적(트레이서) — 발사체에 붙여서 날아가는 동안 따라오게
    if (TracerEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            TracerEffect, CollisionComp, NAME_None,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget, true);
    }
}

void ANCProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                           UPrimitiveComponent* OtherComp, FVector /*NormalImpulse*/,
                           const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

    ProcessHit(OtherActor, Hit, Damage);

    if (OtherActor->IsA<ACharacter>() && TryPenetrate(OtherActor))
        return;

    Destroy();
}

bool ANCProjectile::TryPenetrate(AActor* OtherActor)
{
    if (PenetrationsRemaining <= 0) return false;

    --PenetrationsRemaining;
    Damage = FMath::RoundToFloat(Damage * (1.f - PenetrationDamageFalloff));

    CollisionComp->IgnoreActorWhenMoving(OtherActor, true);
    MovementComp->Velocity = GetActorForwardVector() * ProjectileSpeed;

    return true;
}

void ANCProjectile::ResolveTracePenetration(float InMaxRange)
{
    AActor* OwnerActor = GetOwner();
    const FVector Start = GetActorLocation();
    const FVector End   = Start + GetActorForwardVector() * InMaxRange;
    const float Radius  = CollisionComp->GetScaledSphereRadius();

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwnerActor) Params.AddIgnoredActor(OwnerActor);
    Params.bTraceComplex = false;

    float BlockDistance = InMaxRange;
    {
        FCollisionQueryParams WallParams = Params;
        FCollisionObjectQueryParams WallObjParams;
        WallObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
        WallObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

        constexpr int32 MaxIgnoreIterations = 8;
        for (int32 i = 0; i < MaxIgnoreIterations; ++i)
        {
            FHitResult WallHit;
            if (!GetWorld()->LineTraceSingleByObjectType(WallHit, Start, End, WallObjParams, WallParams))
                break;

            AActor* WallActor = WallHit.GetActor();
            if (WallActor && WallActor->IsA<ANCItemActor>())
            {
                WallParams.AddIgnoredActor(WallActor);
                continue;
            }

            BlockDistance = WallHit.Distance;
            break;
        }
    }

    const FVector PawnEnd = Start + GetActorForwardVector() * BlockDistance;
    FCollisionObjectQueryParams PawnObjParams;
    PawnObjParams.AddObjectTypesToQuery(ECC_Pawn);

    TArray<FHitResult> Hits;
    GetWorld()->SweepMultiByObjectType(
        Hits, Start, PawnEnd, FQuat::Identity, PawnObjParams, FCollisionShape::MakeSphere(Radius), Params);

    Hits.Sort([](const FHitResult& A, const FHitResult& B) { return A.Distance < B.Distance; });

    for (const FHitResult& Hit : Hits)
    {
        AActor* OtherActor = Hit.GetActor();
        if (!OtherActor || OtherActor == OwnerActor || OtherActor->IsA<ANCProjectile>()) continue;

        if (!OtherActor->IsA<ACharacter>())
            continue;

        const float DamageSnapshot = Damage;
        const float DelaySec = ProjectileSpeed > 0.f ? FMath::Max(Hit.Distance / ProjectileSpeed, 0.001f) : 0.001f;
        const FHitResult HitCopy = Hit;
        TWeakObjectPtr<ANCProjectile> WeakThis(this);
        TWeakObjectPtr<AActor> WeakOther(OtherActor);

        FTimerHandle Handle;
        GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda(
            [WeakThis, WeakOther, HitCopy, DamageSnapshot]()
            {
                if (WeakThis.IsValid() && WeakOther.IsValid())
                    WeakThis->ProcessHit(WeakOther.Get(), HitCopy, DamageSnapshot);
            }), DelaySec, false);

        if (!TryPenetrate(OtherActor))
            break;
    }
}

bool ANCProjectile::CheckPointBlankOverlap()
{
    AActor* OwnerActor = GetOwner();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwnerActor) Params.AddIgnoredActor(OwnerActor);

    GetWorld()->OverlapMultiByObjectType(
        Overlaps, GetActorLocation(), FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(PointBlankCheckRadius), Params);

    const FVector Origin = GetActorLocation();
    Overlaps.Sort([Origin](const FOverlapResult& A, const FOverlapResult& B)
    {
        const AActor* ActorA = A.GetActor();
        const AActor* ActorB = B.GetActor();
        const float DistA = ActorA ? FVector::DistSquared(Origin, ActorA->GetActorLocation()) : TNumericLimits<float>::Max();
        const float DistB = ActorB ? FVector::DistSquared(Origin, ActorB->GetActorLocation()) : TNumericLimits<float>::Max();
        return DistA < DistB;
    });

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor || OtherActor == OwnerActor || OtherActor->IsA<ANCProjectile>()) continue;

        FHitResult Hit;
        Hit.ImpactPoint = GetActorLocation();
        Hit.ImpactNormal = -GetActorForwardVector();
        Hit.Component = Overlap.GetComponent();

        ProcessHit(OtherActor, Hit, Damage);

        if (OtherActor->IsA<ACharacter>() && TryPenetrate(OtherActor))
            return false;

        Destroy();
        return true;
    }

    return false;
}

void ANCProjectile::ProcessHit(AActor* OtherActor, const FHitResult& Hit, float InDamage)
{
    //헌호수정 - 바렛 스나이퍼 킬: 좀비면 일반 데미지 대신 "정지→순서대로 지연 죽음" 예약하고 종료
    // (바렛 아니거나 좀비 아니면 이 블록 건너뛰고 아래 기존 로직 그대로 실행)
    if (bSniperKill)
    {
        if (AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(OtherActor))
        {
            Monster->SetLastDamageCauser(GetInstigator());   // 점수/콤보 귀속 유지
            Monster->TriggerSniperKill(SniperKillBaseDelay + SniperHitOrder * SniperKillStagger);
            ++SniperHitOrder;                                 // 다음 관통 좀비는 더 늦게 터짐(도미노)
            return;                                           // 일반 데미지/이펙트 스킵
        }
    }

    // GAS 데미지 적용
    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

    if (SourceASC && TargetASC)
    {
        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        Context.AddHitResult(Hit);  // BoneName → 좀비 쪽 ClassifyBodyPart가 부위 배율 적용
        FGameplayEffectSpecHandle    Spec    = SourceASC->MakeOutgoingSpec(
            UGE_Damage::StaticClass(), 1.f, Context);

        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(NCData::Damage, -InDamage);
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
        }
    }

    const bool bIsCharacter = OtherActor->IsA<ACharacter>();

    if (bIsCharacter)
    {
        // 좀비/캐릭터 피격 — 기존 피격 이펙트(나이아가라 우선, 없으면 파티클)
        if (ImpactFleshEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                this, ImpactFleshEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
        }
        else if (ImpactFleshParticle)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                this, ImpactFleshParticle, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
        }

        if (HitShakeClass)
        {
            if (APlayerController* PC = Cast<APlayerController>(GetInstigatorController()))
                PC->ClientStartCameraShake(HitShakeClass, HitShakeScale);
        }
    }
    else if (SourceASC)
    {
        // 표면(벽 등) 피격 — 표면별 임팩트 큐 실행
        // GC_SurfaceImpact가 PhysMaterial → SurfaceType → DA_Impact 조회해서 콘크리트/메탈/유리 이펙트 분기
        static const FGameplayTag SurfaceTag =
            FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Gun.Surface"));

        FGameplayCueParameters CueParams;
        CueParams.Location         = Hit.ImpactPoint;
        CueParams.Normal           = Hit.ImpactNormal;
        CueParams.PhysicalMaterial = Hit.PhysMaterial;

        SourceASC->ExecuteGameplayCue(SurfaceTag, CueParams);
    }
}
