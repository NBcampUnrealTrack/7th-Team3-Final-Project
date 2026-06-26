#include "NCProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" // PrintString
#include "PhysicalMaterials/PhysicalMaterial.h" // 디버그용 SurfaceType
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Common/NCGameplayTags.h"
#include "GAS/Effect/GE_Damage.h"
#include "GameplayEffectTypes.h" // FGameplayCueParameters

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

    InitialLifeSpan = 3.f;
}

void ANCProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* OwnerActor = GetOwner())
        CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);

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

    UE_LOG(LogTemp, Warning, TEXT("[Projectile] OnHit: Other=%s"), *GetNameSafe(OtherActor));

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

        // 디버그: 어떤 PhysMaterial / SurfaceType을 읽었는지 확인
        const UPhysicalMaterial* PM = Hit.PhysMaterial.Get();
        const FString PMName = GetNameSafe(PM);
        const int32 SurfaceVal = PM ? (int32)PM->SurfaceType.GetValue() : -1;
        UE_LOG(LogTemp, Warning, TEXT("[Projectile] Surface hit: PhysMat=%s, SurfaceType=%d"), *PMName, SurfaceVal);
        UKismetSystemLibrary::PrintString(this,
            FString::Printf(TEXT("PhysMat=%s  Surface=%d"), *PMName, SurfaceVal));

        SourceASC->ExecuteGameplayCue(SurfaceTag, CueParams);
    }

    Destroy();
}
