#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShakeBase.h"

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

    // 총알 궤적 — 발사체에 붙어서 날아가는 동안 따라오는 트레일
    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Niagara")
    TObjectPtr<UNiagaraSystem> TracerEffect;

    // 파티클 이펙트 (나이아가라 미설정 시)
    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Particle")
    TObjectPtr<UParticleSystem> ImpactFleshParticle;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile|Effect|Particle")
    TObjectPtr<UParticleSystem> ImpactSurfaceParticle;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    TSubclassOf<UCameraShakeBase> HitShakeClass;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    float HitShakeScale = 1.f;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    int32 PenetrationsRemaining = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile")
    float PenetrationDamageFalloff = 0.3f;

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

    void ProcessHit(AActor* OtherActor, const FHitResult& Hit);
    bool CheckPointBlankOverlap();

    bool TryPenetrate(AActor* OtherActor);
};
