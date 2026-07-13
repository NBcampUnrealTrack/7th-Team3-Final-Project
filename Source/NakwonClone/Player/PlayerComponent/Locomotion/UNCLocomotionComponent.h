#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UNCLocomotionComponent.generated.h"

class ACharacter;
class ANCBaseCharacter;
class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCLocomotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCLocomotionComponent();

protected:
	virtual void BeginPlay() override;

public:
	//걷기/뛰기/달리기 상태 변경
	void SetGaitTag(FGameplayTag NewGaitTag);

	//앉기 토글
	void SetStanceTag(FGameplayTag NewStanceTag);

	//현재 GaitTag 반환 (AnimInstance에서 읽음)
	FGameplayTag GetCurrentGaitTag() const { return CurrentGaitTag; }
	FGameplayTag GetCurrentStanceTag() const { return CurrentStanceTag; }

private:
	//캐싱 (매 틱마다 GetOwner 캐스팅 방지 → 최적화)
	UPROPERTY()
	TObjectPtr<ANCBaseCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

	//현재 상태 태그
	FGameplayTag CurrentGaitTag;
	FGameplayTag CurrentStanceTag;

	// 이동속도 DataTable (에디터에서 연결)
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> MovementDataTable;

	//DataTable에서 속도 가져와서 적용
	void ApplyMovementSpeed();

	//헌호수정 - 스태미나 시스템
public:
	void StartStaminaDrain();
	void StopStaminaDrain();
	void ClearAllStaminaTimers();
	bool IsSprintLocked() const { return bSprintLocked; }

private:
	FTimerHandle StaminaDrainHandle;
	FTimerHandle StaminaRegenHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaDrainRate = 8.f; //헌호수정

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaRegenRate = 5.f; //헌호수정

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaRegenThreshold = 15.f; //헌호수정

	bool bSprintLocked = false;

	void DrainStamina();
	void RegenStamina();
	void OnStaminaEmpty();
	
#pragma region 시환 추가
	
public:
	void ActivateSpeedBoost(float Multiplier, float Duration);

private:
	float SpeedBoostMultiplier = 1.f;
	FTimerHandle SpeedBoostTimerHandle;
	
public:
	void EndSpeedBoost();
#pragma endregion
};