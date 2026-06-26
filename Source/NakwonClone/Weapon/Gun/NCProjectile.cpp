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

ANCProjectile::ANCProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComp->OnComponentHit.AddDynamic(this, &ANCProjectile::OnHit);
    RootComponent = CollisionComp;

    MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    MovementComp->UpdatedComponent  = CollisionComp;
    MovementComp->InitialSpeed       = 10000.f;
    MovementComp->MaxSpeed           = 10000.f;
    MovementComp->bRotationFollowsVelocity = true;
    MovementComp->bShouldBounce      = false;

    InitialLifeSpan = 3.f;
}

void ANCProjectile::BeginPlay()
{
    Super::BeginPlay();

    // ProjectileSpeed는 GunComponent가 SpawnActor 직후 설정
    MovementComp->InitialSpeed = ProjectileSpeed;
    MovementComp->MaxSpeed     = ProjectileSpeed;
    if (ProjectileSpeed > 0.f)
        SetLifeSpan(MaxRange / ProjectileSpeed);
}

void ANCProjectile::OnHit(UPrimitiveComponent* /*HitComp*/, AActor* OtherActor,
                           UPrimitiveComponent* /*OtherComp*/, FVector /*NormalImpulse*/,
                           const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == GetOwner()) return;

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

    // 피격 대상이 캐릭터(좀비)면 피격 이펙트, 아니면 표면 이펙트
    // 나이아가라 우선 미설정 시 파티클
    const bool bIsCharacter = OtherActor->IsA<ACharacter>();

    UNiagaraSystem* NiagaraFX = bIsCharacter ? ImpactFleshEffect.Get() : ImpactSurfaceEffect.Get();
    if (NiagaraFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, NiagaraFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
    }
    else
    {
        UParticleSystem* ParticleFX = bIsCharacter ? ImpactFleshParticle.Get() : ImpactSurfaceParticle.Get();
        if (ParticleFX)
            UGameplayStatics::SpawnEmitterAtLocation(
                this, ParticleFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
    }

    Destroy();
}
