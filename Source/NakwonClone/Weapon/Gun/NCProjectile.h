#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"

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

    UPROPERTY(BlueprintReadWrite, Category = "Projectile|FX")
    TObjectPtr<UNiagaraSystem> ImpactFleshEffect;

    UPROPERTY(BlueprintReadWrite, Category = "Projectile|FX")
    TObjectPtr<UNiagaraSystem> ImpactSurfaceEffect;

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
