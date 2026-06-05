#include "NCMeleeWeapon.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "DrawDebugHelpers.h"

ANCMeleeWeapon::ANCMeleeWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ANCMeleeWeapon::BeginPlay()
{
    Super::BeginPlay();
}

void ANCMeleeWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //서버에서만 트레이스 실행
    if (HasAuthority() && bIsTracing)
    {
        PerformTrace();
    }
}

void ANCMeleeWeapon::InitFromDataTable(FName RowName)
{
    if (!WeaponDataTable) return;

    FNCWeaponData* Data = WeaponDataTable->FindRow<FNCWeaponData>(
        RowName, TEXT("MeleeWeaponData"));

    if (Data)
    {
        CurrentWeaponData = *Data;
    }
}

void ANCMeleeWeapon::StartTrace()
{
    //AnimNotify는 서버에서만 실행되도록
    if (!HasAuthority()) return;

    HitActors.Empty();
    bIsTracing = true;

    //첫 프레임 위치 초기화
    PreviousStart = WeaponMesh->GetSocketLocation(FName("WeaponStart"));
    PreviousEnd   = WeaponMesh->GetSocketLocation(FName("WeaponEnd"));
}

void ANCMeleeWeapon::EndTrace()
{
    if (!HasAuthority()) return;

    bIsTracing = false;
    HitActors.Empty();
}

void ANCMeleeWeapon::PerformTrace()
{
    FVector CurrentStart = WeaponMesh->GetSocketLocation(FName("WeaponStart"));
    FVector CurrentEnd   = WeaponMesh->GetSocketLocation(FName("WeaponEnd"));

    for (int32 i = 0; i <= 3; i++)
    {
        float Alpha = i / 3.f;

        FVector PrevPoint    = FMath::Lerp(PreviousStart, PreviousEnd, Alpha);
        FVector CurrentPoint = FMath::Lerp(CurrentStart, CurrentEnd, Alpha);

        TArray<FHitResult> HitResults;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(6.f);

        bool bHit = GetWorld()->SweepMultiByChannel(
            HitResults,
            PrevPoint,
            CurrentPoint,
            FQuat::Identity,
            ECC_Pawn,
            Sphere
        );

        if (bHit)
        {
            for (FHitResult& Hit : HitResults)
            {
                AActor* HitActor = Hit.GetActor();
                if (!HitActor) continue;
                if (HitActor == GetOwner()) continue;
                if (HitActors.Contains(HitActor)) continue;

                if (ANCBaseCharacter* Target = Cast<ANCBaseCharacter>(HitActor))
                {
                    HitActors.Add(HitActor);
                    Server_ApplyDamage(Target);
                }
            }
        }
    }

    //디버그를 모든 클라이언트에서 보이도록 Multicast
    Multicast_DrawDebug(CurrentStart, CurrentEnd);

    PreviousStart = CurrentStart;
    PreviousEnd   = CurrentEnd;
}

//모든 클라이언트에서 디버그 표시
void ANCMeleeWeapon::Multicast_DrawDebug_Implementation(FVector Start, FVector End)
{
#if WITH_EDITOR
    // 트레이스 라인
    DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.5f, 0, 2.f);

    // 시작점/끝점 구체
    DrawDebugSphere(GetWorld(), Start, 6.f, 8, FColor::Green, false, 0.5f);
    DrawDebugSphere(GetWorld(), End,   6.f, 8, FColor::Blue,  false, 0.5f);
#endif
}

void ANCMeleeWeapon::Server_ApplyDamage_Implementation(ANCBaseCharacter* Target)
{
    if (!Target) return;
    if (!DamageEffectClass) return;

    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    UAbilitySystemComponent* SourceASC = GetOwner() ?
        Cast<ANCBaseCharacter>(GetOwner())->GetAbilitySystemComponent() : nullptr;

    if (TargetASC && SourceASC)
    {
        FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
        // DataTable의 Damage 값 적용
        FGameplayEffectSpecHandle EffectSpec = SourceASC->MakeOutgoingSpec(
            DamageEffectClass, CurrentWeaponData.Damage, EffectContext);

        if (EffectSpec.IsValid())
        {
            SourceASC->ApplyGameplayEffectSpecToTarget(
                *EffectSpec.Data.Get(), TargetASC);
        }
    }
}