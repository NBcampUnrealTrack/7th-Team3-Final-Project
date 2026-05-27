#include "NCGameplayTags.h"

// 양식: UE_DEFINE_GAMEPLAY_TAG(네임스페이스::변수명, "에디터에_보여질_태그_이름");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Walk, "Character.State.Walk");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Jog, "Character.State.Jog");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Sprint, "Character.State.Sprint");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Attacking, "Character.Action.Attacking");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Stand, "Character.State.Standing");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Landed, "Character.State.Landed");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Crouch, "Character.State.Crouch");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Alive, "Character.State.Alive");;
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Downed, "Character.State.Downed");;
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Dead, "Character.State.Dead");