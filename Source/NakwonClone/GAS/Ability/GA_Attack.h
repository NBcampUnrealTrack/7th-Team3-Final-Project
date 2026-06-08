#pragma once

#include "CoreMinimal.h"
#include "NCGameplayAbilityBase.h"
#include "GA_Attack.generated.h"

UCLASS()
class NAKWONCLONE_API UGA_Attack : public UNCGameplayAbilityBase
{
    GENERATED_BODY()

public:
    UGA_Attack();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

private:
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
