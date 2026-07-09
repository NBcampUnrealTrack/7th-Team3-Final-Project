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

    if (CheckPointBlankOverlap()) return;

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

void ANCProjectile::OnHit(UPrimitiveComponent* /*HitComp*/, AActor* OtherActor,
                           UPrimitiveComponent* /*OtherComp*/, FVector /*NormalImpulse*/,
                           const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

    ProcessHit(OtherActor, Hit);
    Destroy();
}

bool ANCProjectile::CheckPointBlankOverlap()
{
    AActor* OwnerActor = GetOwner();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (OwnerActor) Params.AddIgnoredActor(OwnerActor);

    const float Radius = CollisionComp->GetScaledSphereRadius();
    GetWorld()->OverlapMultiByChannel(
        Overlaps, GetActorLocation(), FQuat::Identity,
        ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor || OtherActor == OwnerActor) continue;

        FHitResult Hit;
        Hit.ImpactPoint = GetActorLocation();
        Hit.ImpactNormal = -GetActorForwardVector();
        Hit.Component = Overlap.GetComponent();

        ProcessHit(OtherActor, Hit);
        Destroy();
        return true;
    }

    return false;
}

void ANCProjectile::ProcessHit(AActor* OtherActor, const FHitResult& Hit)
{
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
            Spec.Data->SetSetByCallerMagnitude(NCData::Damage, -Damage);
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
