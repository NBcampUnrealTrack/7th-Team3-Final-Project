#include "NCFlamethrowerItemActor.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Player/PlayerComponent/NCFlamethrowerComponent.h"

void ANCFlamethrowerItemActor::Interact_Implementation(AActor* Interactor)
{
    ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(Interactor);
    if (!Player) return;

    UNCFlamethrowerComponent* FlamethrowerComp = Player->GetFlamethrowerComponent();
    if (!FlamethrowerComp) return;

    FlamethrowerComp->EquipFlamethrower(Duration);

    ConsumeItem();
}
