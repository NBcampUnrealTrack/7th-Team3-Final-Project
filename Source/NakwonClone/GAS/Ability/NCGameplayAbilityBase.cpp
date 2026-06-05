#include "NCGameplayAbilityBase.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "NakwonClone/Player/PlayerComponent/Combat/UNCCombatComponent.h"

ANCBaseCharacter* UNCGameplayAbilityBase::GetOwnerCharacter() const
{
	return Cast<ANCBaseCharacter>(GetAvatarActorFromActorInfo());
}

UNCCombatComponent* UNCGameplayAbilityBase::GetCombatComponent() const
{
	ANCBaseCharacter* Character = GetOwnerCharacter();
	if (!Character) return nullptr;

	return Character->FindComponentByClass<UNCCombatComponent>();
}