#include "NCProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
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

    UE_LOG(LogTemp, Warning, TEXT("[Projectile] OnHit: %s"), *OtherActor->GetName());

    // GAS 데미지 적용
    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

    UE_LOG(LogTemp, Warning, TEXT("[Projectile] SourceASC: %s / TargetASC: %s"),
        SourceASC ? TEXT("OK") : TEXT("NULL"),
        TargetASC ? TEXT("OK") : TEXT("NULL"));

    if (SourceASC && TargetASC)
    {
        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        FGameplayEffectSpecHandle    Spec    = SourceASC->MakeOutgoingSpec(
            UGE_Damage::StaticClass(), 1.f, Context);

        if (Spec.IsValid())
        {
            // TODO: 현준님에게 좀비 스켈레톤 본 이름(머리/팔/몸통/다리) 전달받으면
            //       Hit.BoneName 기반 부위별 데미지 배율 로직 추가 예정
            Spec.Data->SetSetByCallerMagnitude(NCData::Damage, -Damage);
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
        }
    }

    // 피격 대상이 캐릭터(좀비)면 피격 이펙트, 아니면 표면 이펙트
    const bool bIsCharacter = OtherActor->IsA<ACharacter>();
    UNiagaraSystem* FX = bIsCharacter ? ImpactFleshEffect.Get() : ImpactSurfaceEffect.Get();
    if (FX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this, FX,
            Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation());
    }

    Destroy();
}
