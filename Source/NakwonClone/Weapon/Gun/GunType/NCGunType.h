#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "NCGunType.generated.h"

class ANCProjectile;
class USoundBase;

UENUM(BlueprintType)
enum class ENCGunSlot : uint8
{
    Primary,    // 1번 — 라이플/샷건
    Secondary,  // 2번 — 권총/리볼버
    None
};

UENUM(BlueprintType)
enum class ENCFireMode : uint8
{
    SemiAuto,   // 단발
    FullAuto    // 연사
};

USTRUCT(BlueprintType)
struct FNCGunData : public FTableRowBase
{
    GENERATED_BODY()

    // 슬롯 & 타입
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ENCGunSlot   SlotType      = ENCGunSlot::Primary;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ENCFireMode  DefaultFireMode = ENCFireMode::SemiAuto;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool         bCanToggleFireMode = false;  // 연사<->단발 전환 가능 여부
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag GunTypeTag;

    // UI
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;

    // 전투 수치
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Damage          = 30.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float FireRate        = 10.f;   // 연사 시 초당 발사 수
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ProjectileSpeed = 10000.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MaxRange        = 5000.f; // 발사체 최대 사거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 MagazineSize    = 30;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 MaxReserveAmmo  = 90;

    // 샷건 전용
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 NumPellets   = 1;    // 샷건 펠릿 수량 조절용
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float SpreadAngle  = 0.f;  // 탄퍼짐

    // 재장전
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ReloadTime = 2.f;

    // ADS(정조준)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ADSFOVMultiplier = 0.6f;

    // 소켓
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName MuzzleSocketName = TEXT("Muzzle");
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName HandSocketName   = TEXT("hand_rSocket");

    // 사운드
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> FireSound;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> ReloadSound;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase> EmptyClickSound;

    // 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UNiagaraSystem> MuzzleFlashEffect;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UNiagaraSystem> ImpactFleshEffect;   // 좀비 피격
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UNiagaraSystem> ImpactSurfaceEffect; // 벽/바닥 피격
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UNiagaraSystem> ShellCasingEffect;   // 탄피 배출

    // 탄피 소켓
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName EjectSocketName = TEXT("Eject");

    // 애니메이션 에셋
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UStaticMesh>   GunMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<ANCProjectile>    ProjectileClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage>  FireMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage>  ReloadMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage>  ADSInMontage;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage>  ADSOutMontage;
};

USTRUCT(BlueprintType)
struct FNCGunSlotData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName GunID;
    UPROPERTY(BlueprintReadOnly) int32 CurrentAmmo  = 0;
    UPROPERTY(BlueprintReadOnly) int32 ReserveAmmo  = 0;

    bool IsEmpty() const { return GunID.IsNone(); }
    void Clear() { GunID = NAME_None; CurrentAmmo = 0; ReserveAmmo = 0; }
};
