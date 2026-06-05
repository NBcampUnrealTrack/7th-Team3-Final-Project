#include "GA_Attack.h"
#include "NakwonClone/Player/PlayerComponent/Combat/UNCCombatComponent.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "AbilitySystemComponent.h"

UGA_Attack::UGA_Attack()
{
    //서버에서 실행 후 클라이언트에 통보
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Attack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    //CombatComponent 가져오기
    UNCCombatComponent* Combat = GetCombatComponent();
    if (!Combat || !Combat->CanAttack())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //무기 데이터 가져오기
    FNCWeaponData* WeaponData = Combat->GetEquippedWeaponData();
    if (!WeaponData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //공격 태그 추가
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
        ASC->AddLooseGameplayTag(NCWeapon::Action_Attacking);

    //몽타주 재생
    UAnimMontage* Montage = WeaponData->AttackMontage.LoadSynchronous();
    if (Montage)
    {
        UAnimInstance* AnimInstance = 
            ActorInfo->GetAnimInstance();

        if (AnimInstance)
        {
            AnimInstance->Montage_Play(Montage, WeaponData->AttackSpeed);
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &UGA_Attack::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
        }
    }

    //내구도 감소
    Combat->ReduceDurability(1.f);
}

void UGA_Attack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    //공격 태그 제거
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
        ASC->RemoveLooseGameplayTag(NCWeapon::Action_Attacking);

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true, bInterrupted);
}

void UGA_Attack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, 
        bReplicateEndAbility, bWasCancelled);
}