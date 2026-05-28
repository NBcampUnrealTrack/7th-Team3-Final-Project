#include "UNCStatComponent.h"
#include "UNCStatComponent.h"
#include "Net/UnrealNetwork.h"

UNCStatComponent::UNCStatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true); // 멀티플레이 복제 활성화

    CurrentHP = 100.f;
    MaxHP = 100.f;
    MaxStamina = 100.f;
    bIsAlive = true;
}

void UNCStatComponent::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 데이터 테이블 읽기
    if (GetOwner()->HasAuthority())
    {
        if (StatDataTable)
        {
            FNCPlayerStatData* StatData = StatDataTable->FindRow<FNCPlayerStatData>(
                FName("1"), TEXT("StatLookup"));

            if (StatData)
            {
                MaxHP = StatData->MaxHP;
                MaxStamina = StatData->MaxStamina;
                CurrentHP = MaxHP;
                bIsAlive = StatData->bIsAlive;
            }
        }
    }
}

void UNCStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UNCStatComponent, CurrentHP);
    DOREPLIFETIME(UNCStatComponent, MaxHP);
    DOREPLIFETIME(UNCStatComponent, MaxStamina);
    DOREPLIFETIME(UNCStatComponent, bIsAlive);
}

void UNCStatComponent::ApplyDamage(float DamageAmount)
{
    if (!GetOwner()->HasAuthority()) return; // 서버에서만 처리
    if (!bIsAlive) return;

    float OldHP = CurrentHP;
    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

    OnHPChanged.Broadcast(OldHP, CurrentHP);

    if (CurrentHP <= 0.f)
    {
        HandleDeath();
    }
}

void UNCStatComponent::ApplyHeal(float HealAmount)
{
    if (!GetOwner()->HasAuthority()) return; // 서버에서만 처리
    if (!bIsAlive) return;

    float OldHP = CurrentHP;
    CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.f, MaxHP);

    OnHPChanged.Broadcast(OldHP, CurrentHP);
}

void UNCStatComponent::OnRep_CurrentHP(float OldHP)
{
    // 클라이언트에서 HP 변경 감지 → UI 업데이트 등
    OnHPChanged.Broadcast(OldHP, CurrentHP);
}

void UNCStatComponent::HandleDeath()
{
    bIsAlive = false;
    OnDead.Broadcast();
}