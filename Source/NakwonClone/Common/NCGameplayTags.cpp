#include "NCGameplayTags.h"

// 양식: UE_DEFINE_GAMEPLAY_TAG(네임스페이스::변수명, "에디터에_보여질_태그_이름");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Walk, "Character.State.Walk");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Jog, "Character.State.Jog");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Sprint, "Character.State.Sprint");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Attacking, "Character.Action.Attacking");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Stand, "Character.State.Standing");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Landed, "Character.State.Landed");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Crouch, "Character.State.Crouch");

UE_DEFINE_GAMEPLAY_TAG(NCItemType::Equipment, "ItemType.Equipment")
UE_DEFINE_GAMEPLAY_TAG(NCItemType::Consumable, "ItemType.Consumable")
UE_DEFINE_GAMEPLAY_TAG(NCItemType::Material, "ItemType.Material")

UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Eating, "Item.Consumable.Eating")
UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Heal, "Item.Consumable.Heal")
UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Credit, "Item.Currency.Credit")
UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Quest, "Item.Currency.Quest")

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Alive, "Character.State.Alive");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Downed, "Character.State.Downed");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Dead, "Character.State.Dead");

// 게임 상태
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::Title, "GameState.State.Title");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::Lobby, "GameState.State.Lobby");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::GameStart, "GameState.State.GameStart");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::GameOver, "GameState.State.GameOver");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::GameClear, "GameState.State.Clear");
	
// 싱글, 멀티 여부
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::SinglePlay, "GameState.State.SinglePlay");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::MultiPlay, "GameState.State.MultiPlay");