#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/*
 * [새로운 태그 추가하는 방법]
 * 관련된 성격의 namespace(네임스페이스)를 찾거나 새로 만들기
 * 네임스페이스 내부에 아래 예시 매크로를 선언
 * 예시: UE_DECLARE_GAMEPLAY_TAG_EXTERN(MyNewState);
 * 소스(.cpp) 파일로 이동해서 실제 문자열을 매핑
 */

// 캐릭터 상태 및 액션 태그
namespace NCCharacter
{
	/* 
	 * [다른 C++ 파일에서 사용하는 방법]
	 * 사용하려는 파일 상단에 #include "Common/NCGameplayTags.h" 를 추가
	 * 아래 예시 코드와 같이 네임스페이스와 변수명을 조합해 사용
	 * 예시 코드:
	 * if (CurrentMovementState == EMovementState::Sprint)
	 * {
	 * // 캐릭터에게 달리기 태그 부여
	 * GameplayTagContainer.AddTag(NCCharacter::Sprint);
	 * }
	 */
	
	// 캐릭터 종류
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Monster);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Monster_Walker);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Monster_Runner);
	
	// 이동 관련
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Walk);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jog);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CrouchSprint); //헌호수정

	// 상태 관련
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stand);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Landed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crouch);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InventoryOpen);
	
	// 생존 관련
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Alive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Downed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
};

namespace NCItemType
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equipment); // 장비 아이템
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consumable); // 소모품
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Material); // 재료
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Currency); // 화폐
};

namespace NCItemTag
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon);  // 무기
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Armor);   // 방어구
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Food);  // 음식 아이템
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Heal);    // 회복 아이템
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Credit);  // 크레딧
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Quest);   // 퀘스트 아이템
};

namespace NCLootBox
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BeingLooted); // 누군가 파밍 중인 상태
};

// 무기 관련 태그
namespace NCWeapon
{
	// 무기 타입
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Type_Unarmed);    // 맨손
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Type_OneHanded);  // 한손 (크로우바, 칼)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Type_TwoHanded);  // 양손 (도끼, 배트)

	// 무기 무게
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weight_Light);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weight_Medium);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weight_Heavy);

	// 무기 액션
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Attacking);   // 공격 중
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_SwingDelay);  // 공격 후 딜레이
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Swapping);    // 무기 교체 중

	// 무기 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Equipped);  // 장착됨
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Broken);    // 파손
};

// 데미지 데이터 태그 (SetByCaller용)
namespace NCData
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage);
};

namespace NCGameStateTags
{
	// 게임 상태
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Title);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Lobby);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameOver);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameClear);
	
	// 싱글, 멀티 여부
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SinglePlay);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(MultiPlay);
};