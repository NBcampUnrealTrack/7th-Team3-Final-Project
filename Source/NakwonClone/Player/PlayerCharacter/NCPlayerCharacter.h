#pragma once

#include "CoreMinimal.h"
#include "NCBaseCharacter.h"
#include "NakwonClone/GAS/Ability/GA_Attack.h"
#include "NCPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ACameraActor;
class UNCPlayerInventoryComponent;
class UNCLocomotionComponent;
class UNCCombatComponent;
class UNCGunComponent;
class USpotLightComponent;
class UStaticMeshComponent;
class UUserWidget;

UCLASS()
class NAKWONCLONE_API ANCPlayerCharacter : public ANCBaseCharacter
{
	GENERATED_BODY()

public:
	ANCPlayerCharacter();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Ability")
	TSubclassOf<UGA_Attack> AttackAbilityClass;

	UFUNCTION(BlueprintPure, Category = "Components|Combat")
	UNCCombatComponent* GetCombatComponent() const { return CombatComponent; }

	// 하상빈 추가
	UFUNCTION(BlueprintPure, Category = "Components|Gun")
	UNCGunComponent* GetGunComponent() const { return GunComponent; }

	// 근접무기 슬롯 — 줍는 순간 저장, 3번 키로 꺼냄 (임시)
	UPROPERTY(BlueprintReadWrite, Category = "Components|Combat")
	FName StoredMeleeWeaponID;

	// 드랍 시 스폰할 픽업 액터 클래스
	UPROPERTY(BlueprintReadWrite, Category = "Components|Combat")
	TSubclassOf<AActor> StoredMeleePickupClass;

	// 처음 주웠을 때 픽업 액터의 회전값 
	UPROPERTY(BlueprintReadWrite, Category = "Components|Combat")
	FRotator StoredMeleePickupRotation;

	FORCEINLINE UNCPlayerInventoryComponent* GetInventoryComponent() const { return PlayerInventoryRef; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer StateTags;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> UseItemMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HealItemMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FoodItemMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> TakeDamageShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> EscapeResultWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void OnUseItemMontageEnded();

	UFUNCTION(BlueprintPure, Category = "Sound|Footstep")
	float GetFootstepVolumeMultiplier() const;

	void StartSprint();
	void StopSprint();
	void ToggleWalk();
	void ToggleCrouch();
	virtual void OnDead() override;

	void ToggleFlashlight();

	// 헌호수정 - 암살 기능
	void TryAssassinate();

	UPROPERTY(EditDefaultsOnly, Category = "Assassination")
	float AssassinationRange = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Animation|HitReact")
	void HandleHitReact(AActor* Attacker);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(Server, Reliable)
	void Server_SetGait(FGameplayTag NewGaitTag);

	UFUNCTION(Server, Reliable)
	void Server_SetStance(FGameplayTag NewStanceTag);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDead();

	UFUNCTION(Server, Reliable)
	void Server_ToggleFlashlight();

	// 헌호수정 - 암살 RPC
	UFUNCTION(Server, Reliable)
	void Server_TryAssassinate();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartAssassination(AVGMonsterCharacterBase* Target);

	AVGMonsterCharacterBase* FindNearestAssassinationTarget() const;
	void StartAssassinationSlowMo();
	void StartAssassinationCamera(AVGMonsterCharacterBase* Target);
	void FinishAssassination();

	UFUNCTION()
	void OnRep_bFlashlightOn();

	void ApplyFlashlightState();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(BlueprintReadOnly, Category = "Components|Inventory")
	TObjectPtr<UNCPlayerInventoryComponent> PlayerInventoryRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Locomotion")
	TObjectPtr<UNCLocomotionComponent> LocomotionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Interaction")
	TObjectPtr<class UNCInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat")
	TObjectPtr<UNCCombatComponent> CombatComponent;

	// 하상빈 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Gun")
	TObjectPtr<UNCGunComponent> GunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Flashlight")
	TObjectPtr<UStaticMeshComponent> FlashlightMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Flashlight")
	TObjectPtr<USpotLightComponent> FlashlightLight;

	UPROPERTY(ReplicatedUsing = OnRep_bFlashlightOn)
	bool bFlashlightOn = false;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|HitReact")
	TObjectPtr<UAnimMontage> HitReactFrontMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|HitReact")
	TObjectPtr<UAnimMontage> HitReactBackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|HitReact")
	TObjectPtr<UAnimMontage> HitReactLeftMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|HitReact")
	TObjectPtr<UAnimMontage> HitReactRightMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|HitReact")
	float HitReactCooldown = 0.5f;

	void HandleHealthChanged(const struct FOnAttributeChangeData& Data);

private:
	void InitCamera();
	void InitComponents();

	UFUNCTION()
	void OnItemUsed(FGameplayTag UsedItemTag);

	UFUNCTION()
	void OnConsumableMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool bCameraShaking = false;
	FTimerHandle ShakeTimerHandle;

	// 헌호수정 - 암살
	FTimerHandle AssassinationTimerHandle;
	FTimerHandle AssassinationCameraTimerHandle;

	UPROPERTY()
	TObjectPtr<class ACameraActor> AssassinationCamera;

	float LastHitReactTime = -999.f;
};