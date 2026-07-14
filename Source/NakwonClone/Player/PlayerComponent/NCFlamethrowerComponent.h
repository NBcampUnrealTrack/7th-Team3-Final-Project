#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NCFlamethrowerComponent.generated.h"

class UMeshComponent;
class UAudioComponent;
class UNiagaraComponent;
class USoundBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCFlamethrowerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNCFlamethrowerComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    float TickInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    float DamagePerTick = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    float ConeAngle = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    float ConeRange = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    TSoftObjectPtr<USkeletalMesh> FlamethrowerSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    TSoftObjectPtr<UStaticMesh> FlamethrowerStaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    FName HandSocketName = TEXT("hand_rSocket");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    FName MuzzleSocketName = TEXT("Muzzle");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    TSoftObjectPtr<UNiagaraSystem> FlameVFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flamethrower")
    TSoftObjectPtr<USoundBase> FireLoopSound;

    bool IsActive() const { return bIsActive; }
    bool IsFiring() const { return bIsFiring; }

    void EquipFlamethrower(float Duration);
    void UnequipFlamethrower();

    void StartFire();
    void StopFire();

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    bool bIsActive = false;
    bool bIsFiring = false;

    UPROPERTY()
    TObjectPtr<UMeshComponent> EquippedMeshComp;

    UPROPERTY()
    TObjectPtr<UAudioComponent> FireLoopAudioComp;

    UPROPERTY()
    TObjectPtr<UNiagaraComponent> FlameVFXComp;

    FTimerHandle DurationTimerHandle;
    FTimerHandle DamageTickTimerHandle;

    void AttachFlamethrowerMesh();
    void DetachFlamethrowerMesh();
    void ApplyConeDamageTick();
    void OnDurationExpired();
};
