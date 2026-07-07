#include "AnimNotify_CanStartNextAttack.h"

#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"

void UAnimNotify_CanStartNextAttack::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}

	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(MeshComp->GetOwner());
	if (!PlayerCharacter)
	{
		return;
	}

	if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
	{
		CombatComp->OpenComboInput();
	}
}