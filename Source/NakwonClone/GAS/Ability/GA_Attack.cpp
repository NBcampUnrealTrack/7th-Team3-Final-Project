#include "GA_Attack.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"

UGA_Attack::UGA_Attack()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Attack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UNCCombatComponent* Combat = GetCombatComponent();
	if (!Combat || !Combat->CanAttack())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!Combat->GetEquippedWeaponData())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->AddLooseGameplayTag(NCWeapon::Action_Attacking);
	}

	if (Combat->IsGunWeapon())
	{
		Combat->GunAttack();
	}
	else
	{
		Combat->MeleeAttack();
	}

	UAnimMontage* Montage = Combat->GetLastPlayedAttackMontage();

	if (!Montage)
	{
		if (ASC)
		{
			ASC->RemoveLooseGameplayTag(NCWeapon::Action_Attacking);
		}

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UGA_Attack::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}

	Combat->ReduceDurability(1.f);
}

void UGA_Attack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Attacking);
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Firing);
	}


	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		true,
		bInterrupted
	);
}

void UGA_Attack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(
		Handle,
		ActorInfo,
		ActivationInfo,
		bReplicateEndAbility,
		bWasCancelled
	);

}