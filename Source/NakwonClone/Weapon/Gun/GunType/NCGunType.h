#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShakeBase.h"
#include "NCGunType.generated.h"

class ANCProjectile;
class ANCGunActor;
class USoundBase;
class UTexture2D;

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
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<UTexture2D> Icon;

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

    // 헌호수정 - 반동
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil") float RecoilPitch         = 1.5f; // 발사당 위로 밀리는 양
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil") float RecoilYaw           = 0.3f; // 발사당 좌우 랜덤 흔들림
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil") float RecoilRecoverySpeed = 5.f;  // 복귀 속도 (초당)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recoil") TSubclassOf<UCameraShakeBase> FireShakeClass; // 발사 시 카메라 쉐이크

    // ADS(정조준)
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ADSFOVMultiplier    = 0.6f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float ADSSpreadMultiplier = 0.3f; // ADS 시 SpreadAngle 배율 (1.0 = 변화 없음)

    // 소켓
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName MuzzleSocketName = TEXT("Muzzle");
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName HandSocketName   = TEXT("hand_rSocket");

    // IK
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
    bool bUseTwoHandIK = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
    bool bUseRightHandIK = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
    FName LeftHandIKSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
    FName RightHandIKSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK")
    bool bUseLeftHandIK = false;

    // 사운드
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound") 
    TSoftObjectPtr<USoundBase> FireSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound") 
    TSoftObjectPtr<USoundBase> ReloadSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
    TSoftObjectPtr<USoundBase> EmptyClickSound;

    // 발사모드 전환 사운드 (bCanToggleFireMode 체크해야 슬롯 노출)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (EditCondition = "bCanToggleFireMode", EditConditionHides))
    TSoftObjectPtr<USoundBase> ToggleFireModeSound;

    // 이펙트 (나이아가라)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NiagaraEffect")
    TSoftObjectPtr<UNiagaraSystem> MuzzleFlashEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NiagaraEffect")
    TSoftObjectPtr<UNiagaraSystem> ImpactFleshEffect;       // 좀비 피격

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NiagaraEffect")
    TSoftObjectPtr<UNiagaraSystem> ImpactSurfaceEffect;     // 벽/바닥 피격

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NiagaraEffect")
    TSoftObjectPtr<UNiagaraSystem> ShellCasingEffect;       // 탄피 배출

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NiagaraEffect")
    TSoftObjectPtr<UNiagaraSystem> TracerEffect;            // 총알 궤적(트레이서)

    // 이펙트 (파티클 - 나이아가라 미설정 시)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParticleEffect")
    TSoftObjectPtr<UParticleSystem> MuzzleFlashParticle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParticleEffect")
    TSoftObjectPtr<UParticleSystem> ImpactFleshParticle;    // 좀비 피격

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParticleEffect")
    TSoftObjectPtr<UParticleSystem> ImpactSurfaceParticle;  // 벽/바닥 피격

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParticleEffect")
    TSoftObjectPtr<UParticleSystem> ShellCasingParticle;    // 탄피 배출

    // 탄피 소켓
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName EjectSocketName = TEXT("Eject");

    // 에셋 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UStaticMesh>   GunMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USkeletalMesh> GunSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ANCProjectile>    ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ANCGunActor>      GunActorClass; // 드롭 시 스폰할 BP 액터 클래스

    // 애니메이션

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage>  FireMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage>  ReloadMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage>  ADSInMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage>  ADSOutMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage>  EquipMontage;    // 총기 꺼낼 때 (1,2번 키)

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage>  UnequipMontage;  // 총기 집어넣을 때 (H키,슬롯 전환)

    // 총기 메시 자체 애니메이션 (SkeletalMesh 전용)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimSequence> GunFireAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimSequence> GunReloadAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    TSoftObjectPtr<UStaticMesh> MagazineDropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Socket")
    FName MagazineSocketName = TEXT("Magazine");
};

USTRUCT(BlueprintType)
struct FNCGunSlotData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName GunID;
    UPROPERTY(BlueprintReadOnly) FGameplayTag GunTypeTag;
    UPROPERTY(BlueprintReadOnly) int32 CurrentAmmo  = 0;
    UPROPERTY(BlueprintReadOnly) int32 ReserveAmmo  = 0;

    bool IsEmpty() const { return GunID.IsNone(); }
    void Clear() { GunID = NAME_None; GunTypeTag = FGameplayTag(); CurrentAmmo = 0; ReserveAmmo = 0; }
};
