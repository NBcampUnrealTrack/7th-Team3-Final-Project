#include "NCGameplayTags.h"

// 양식: UE_DEFINE_GAMEPLAY_TAG(네임스페이스::변수명, "에디터에_보여질_태그_이름");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Player, "Character.Player");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Monster, "Character.Monster");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Monster_Walker, "Character.Monster.Walker");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Monster_Runner, "Character.Monster.Runner");

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Walk, "Character.State.Walk");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Jog, "Character.State.Jog");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Sprint, "Character.State.Sprint");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::CrouchSprint, "Character.State.CrouchSprint"); //헌호수정

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Attacking, "Character.Action.Attacking");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Stand, "Character.State.Standing");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Landed, "Character.State.Landed");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Crouch, "Character.State.Crouch");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::InventoryOpen, "Character.State.InventoryOpen");

// --------------아이템------------------
UE_DEFINE_GAMEPLAY_TAG(NCItemType::Equipment, "Item.Equipment")
UE_DEFINE_GAMEPLAY_TAG(NCItemType::Consumable, "Item.Consumable")
UE_DEFINE_GAMEPLAY_TAG(NCItemType::Material, "Item.Material")
UE_DEFINE_GAMEPLAY_TAG(NCItemType::Currency, "Item.Currency");

UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Weapon, "Item.Equipment.Weapon");
UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Armor, "Item.Equipment.Armor");

UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Food, "Item.Consumable.Food");
UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Heal, "Item.Consumable.Heal");

UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Credit, "Item.Currency.Credit");
UE_DEFINE_GAMEPLAY_TAG(NCItemTag::Quest, "Item.Currency.Quest");

UE_DEFINE_GAMEPLAY_TAG(NCEquip::Hand_Left, "Equip.Hand.Left");

UE_DEFINE_GAMEPLAY_TAG(NCLootBox::State_BeingLooted, "LootBox.State.BeingLooted");
// --------------------------------------

UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Alive, "Character.State.Alive");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Downed, "Character.State.Downed");
UE_DEFINE_GAMEPLAY_TAG(NCCharacter::Dead, "Character.State.Dead");

// 무기 타입
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Type_Unarmed,   "Weapon.Type.Unarmed");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Type_OneHanded, "Weapon.Type.OneHanded");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Type_TwoHanded, "Weapon.Type.TwoHanded");

// 무기 무게
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Weight_Light,  "Weapon.Weight.Light");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Weight_Medium, "Weapon.Weight.Medium");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Weight_Heavy,  "Weapon.Weight.Heavy");

// 무기 액션
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Action_Attacking,  "Weapon.Action.Attacking");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Action_SwingDelay, "Weapon.Action.SwingDelay");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::Action_Swapping,   "Weapon.Action.Swapping");

// 무기 상태
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::State_Equipped, "Weapon.State.Equipped");
UE_DEFINE_GAMEPLAY_TAG(NCWeapon::State_Broken,   "Weapon.State.Broken");

// 데미지 데이터 태그
UE_DEFINE_GAMEPLAY_TAG(NCData::Damage, "Data.Damage");

// 게임 상태
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::Title, "GameState.State.Title");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::Lobby, "GameState.State.Lobby");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::GameStart, "GameState.State.GameStart");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::GameOver, "GameState.State.GameOver");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::GameClear, "GameState.State.Clear");
	
// 싱글, 멀티 여부
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::SinglePlay, "GameState.State.SinglePlay");
UE_DEFINE_GAMEPLAY_TAG(NCGameStateTags::MultiPlay, "GameState.State.MultiPlay")

// 총기 장착 슬롯
UE_DEFINE_GAMEPLAY_TAG(NCGun::Slot_Primary,   "Gun.Slot.Primary");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Slot_Secondary, "Gun.Slot.Secondary");

// 총기 타입
UE_DEFINE_GAMEPLAY_TAG(NCGun::Type_Rifle,   "Gun.Type.Rifle");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Type_Shotgun, "Gun.Type.Shotgun");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Type_Pistol,  "Gun.Type.Pistol");

// 발사 모드
UE_DEFINE_GAMEPLAY_TAG(NCGun::FireMode_Semi, "Gun.FireMode.Semi");
UE_DEFINE_GAMEPLAY_TAG(NCGun::FireMode_Auto, "Gun.FireMode.Auto");

// 총기 액션
UE_DEFINE_GAMEPLAY_TAG(NCGun::Action_Firing,          "Gun.Action.Firing");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Action_Reloading,       "Gun.Action.Reloading");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Action_ADS,             "Gun.Action.ADS");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Action_ToggleFireMode,  "Gun.Action.ToggleFireMode");
UE_DEFINE_GAMEPLAY_TAG(NCGun::Action_Swapping,        "Gun.Action.Swapping");
