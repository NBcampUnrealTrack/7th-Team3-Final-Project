#pragma once

#include "CoreMinimal.h"
#include "NCBaseCharacter.h"
#include "NakwonClone/GAS/Ability/GA_Attack.h"
#include "NCPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNCPlayerInventoryComponent;
class UNCLocomotionComponent;
class UNCCombatComponent;
class USpotLightComponent;
class UStaticMeshComponent;

UCLASS()
class NAKWONCLONE_API ANCPlayerCharacter : public ANCBaseCharacter
{
	GENERATED_BODY()

public:
	ANCPlayerCharacter();
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//GAS 어빌리티
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Ability")
	TSubclassOf<UGA_Attack> AttackAbilityClass;

	UFUNCTION(BlueprintPure, Category = "Components|Combat")
	UNCCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
	//하상빈 추가
	FORCEINLINE UNCPlayerInventoryComponent* GetInventoryComponent() const { return PlayerInventoryRef; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer StateTags;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> UseItemMontage;

	// 헌호수정 - 아이템 종류별 몽타지
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HealItemMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FoodItemMontage;

	// 헌호수정 - 사망 몽타지
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void OnUseItemMontageEnded();

	UFUNCTION(BlueprintPure, Category = "Sound|Footstep")
	float GetFootstepVolumeMultiplier() const;
	
protected:
	virtual void BeginPlay() override;

	//하상빈 추가
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
public:
	void StartSprint();
	void StopSprint();
	void ToggleWalk();
	void ToggleCrouch();
	virtual void OnDead() override;

	// 헌호수정 - 플래시라이트 토글 (T키 입력 시 호출)
	void ToggleFlashlight();

protected:
	UFUNCTION(Server, Reliable)
	void Server_SetGait(FGameplayTag NewGaitTag);

	UFUNCTION(Server, Reliable)
	void Server_SetStance(FGameplayTag NewStanceTag);

	// 헌호수정 - 플래시라이트 서버 RPC
	UFUNCTION(Server, Reliable)
	void Server_ToggleFlashlight();

	// 헌호수정 - 플래시라이트 상태 복제 콜백
	UFUNCTION()
	void OnRep_bFlashlightOn();

	// 헌호수정 - 실제 켜고 끄기 (서버/클라 공통)
	void ApplyFlashlightState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	//하상빈 추가
	UPROPERTY(BlueprintReadOnly, Category = "Components|Inventory")
	TObjectPtr<UNCPlayerInventoryComponent> PlayerInventoryRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Locomotion")
	TObjectPtr<UNCLocomotionComponent> LocomotionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Interaction")
	TObjectPtr<class UNCInteractionComponent> InteractionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat")
	TObjectPtr<UNCCombatComponent> CombatComponent;

	// 헌호수정 - 플래시라이트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Flashlight")
	TObjectPtr<UStaticMeshComponent> FlashlightMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Flashlight")
	TObjectPtr<USpotLightComponent> FlashlightLight;

	// 헌호수정 - 플래시라이트 상태 (Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_bFlashlightOn)
	bool bFlashlightOn = false; //헌호수정
	
	//H 피격 리액션 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayHitReactMontage();

	// 체력 변경 콜백 (피격 감지 → 몽타주 트리거)
	void HandleHealthChanged(const struct FOnAttributeChangeData& Data);

private:
	void InitCamera();
	void InitComponents();

	//하상빈 추가
	UFUNCTION()
	void OnItemUsed(FGameplayTag UsedItemTag);
};