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
	
	// 이동 관련
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Walk);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jog);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sprint);

	// 상태 관련
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stand);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Landed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crouch);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dead);
}