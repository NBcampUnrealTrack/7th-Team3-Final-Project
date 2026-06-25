#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"

#include "NCProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class NAKWONCLONE_API ANCProjectile : public AActor
{
    GENERATED_BODY()

public:
    ANCProjectile();

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    float Damage = 30.f;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    float MaxRange = 5000.f;

    // GunComponent가 SpawnActor 직후 세팅 → BeginPlay에서 MovementComp에 반영
    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    float ProjectileSpeed = 10000.f;

    // 나이아가라 이펙트 (우선 사용)
    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Niagara")
    TObjectPtr<UNiagaraSystem> ImpactFleshEffect;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Niagara")
    TObjectPtr<UNiagaraSystem> ImpactSurfaceEffect;

    // 파티클 이펙트 (나이아가라 미설정 시)
    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Particle")
    TObjectPtr<UParticleSystem> ImpactFleshParticle;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Particle")
    TObjectPtr<UParticleSystem> ImpactSurfaceParticle;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> MovementComp;

    virtual void BeginPlay() override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
               UPrimitiveComponent* OtherComp, FVector NormalImpulse,
               const FHitResult& Hit);
};
