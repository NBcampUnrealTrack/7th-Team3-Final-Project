#include "NCAnimNotify_PickupStore.h"
#include "GameFramework/Actor.h"
#include "NakwonClone/Player/PlayerComponent/NCInteractionComponent.h"

void UNCAnimNotify_PickupStore::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UNCInteractionComponent* InteractionComp = Owner->FindComponentByClass<UNCInteractionComponent>())
	{
		InteractionComp->StorePendingLoot();
	}
}