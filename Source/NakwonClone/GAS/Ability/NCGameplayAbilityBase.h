#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NCGameplayAbilityBase.generated.h"

UCLASS()
class NAKWONCLONE_API UNCGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	//어빌리티를 소유한 캐릭터 반환
	class ANCBaseCharacter* GetOwnerCharacter() const;

	//어빌리티를 소유한 CombatComponent 반환
	class UNCCombatComponent* GetCombatComponent() const;
};