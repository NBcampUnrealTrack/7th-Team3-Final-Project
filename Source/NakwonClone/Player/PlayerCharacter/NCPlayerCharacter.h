#pragma once

#include "CoreMinimal.h"
#include "NCBaseCharacter.h"
#include "NakwonClone/GAS/Ability/GA_Attack.h"
#include "Components/PointLightComponent.h"
#include "NCPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNCPlayerInventoryComponent;
class UNCAssassinationComponent;
class UNCLocomotionComponent;
class UNCCombatComponent;
class UNCGunComponent;
class UNCRifleComponent;
class UNCShotgunComponent;
class UNCPistolComponent;
class UNCEquipmentComponent;
class USpotLightComponent;
class UStaticMeshComponent;
class UVGAttackSlotComponent; 
class UUserWidget;
class UTexture2D;

// 핫바 UI용: 보관 중인 근접무기(StoredMeleeWeaponID)가 바뀔 때 브로드캐스트 (줍기/교체 시)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeleeStoredChanged);

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
	UFUNCTION(BlueprintPure, Category = "Components|Equipment")
	UNCEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	// 현재 활성 무기 컴포넌트 반환 (NCAnimInstance 등 하위호환)
	UFUNCTION(BlueprintPure, Category = "Components|Gun")
	UNCGunComponent* GetGunComponent() const;

	// 근접무기 슬롯 — 줍는 순간 저장, 3번 키로 꺼냄 (임시)
	UPROPERTY(BlueprintReadWrite, Category = "Components|Combat")
	FName StoredMeleeWeaponID;

	// 핫바 아이콘 표시용: 보관 중인 근접무기 아이콘 (없으면 nullptr)
	UFUNCTION(BlueprintCallable, Category = "Components|Combat")
	UTexture2D* GetMeleeIcon() const;

	// 핫바 UI용: 근접무기를 줍거나 교체해서 StoredMeleeWeaponID가 바뀔 때 발생
	UPROPERTY(BlueprintAssignable, Category = "Components|Combat")
	FOnMeleeStoredChanged OnMeleeStoredChanged;

	// 드랍 시 스폰할 픽업 액터 클래스
	UPROPERTY(BlueprintReadWrite, Category = "Components|Combat")
	TSubclassOf<AActor> StoredMeleePickupClass;

	// 처음 주웠을 때 픽업 액터의 회전값 
	UPROPERTY(BlueprintReadWrite, Category = "Components|Combat")
	FRotator StoredMeleePickupRotation;

	UFUNCTION(BlueprintPure, Category = "Components|Inventory")
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

	UFUNCTION(BlueprintCallable, Category = "Animation|HitReact")
	void HandleHitReact(AActor* Attacker);

	virtual void Tick(float DeltaTime) override;

	void SetAimRotationMode(bool bEnable);


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

	// 헌호수정 - 암살 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Assassination")
	TObjectPtr<UNCAssassinationComponent> AssassinationComponent;

	// 하상빈 추가 - 장착 관리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Equipment")
	TObjectPtr<UNCEquipmentComponent> EquipmentComponent;

	// 총기 타입별 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Gun")
	TObjectPtr<UNCRifleComponent> RifleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Gun")
	TObjectPtr<UNCShotgunComponent> ShotgunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Gun")
	TObjectPtr<UNCPistolComponent> PistolComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Flashlight")
	TObjectPtr<UStaticMeshComponent> FlashlightMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Flashlight")
	TObjectPtr<USpotLightComponent> FlashlightLight;

	//헌호수정 - 디비전 스타일 백팩 체력바 (3D 위젯, 등 소켓에 부착)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|HUD")
	TObjectPtr<class UWidgetComponent> BackpackHPWidget;

	//헌호수정 - 백팩 체력바 갱신
	void UpdateBackpackHP();

	//헌호수정 - 정조준(ADS) 시 뜨는 화면 HUD 클래스 (BP에서 WBP_ADSInfo 지정)
	UPROPERTY(EditDefaultsOnly, Category = "Components|HUD")
	TSubclassOf<class UNCADSHUD> ADSHUDWidgetClass;

	//헌호수정 - 생성된 ADS HUD 인스턴스 (한 번 만들고 토글)
	UPROPERTY()
	TObjectPtr<class UNCADSHUD> ADSHUDWidget;

	//헌호수정 - ADS 상태 추적 (전환 감지용)
	bool bWasADS = false;

	//헌호수정 - 백팩/ADS UI 전환 갱신
	void UpdateWeaponHUDs();

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

	//헌호수정 - 피격 스턴 (좀비 위협도 강화: 맞으면 하던 행동 취소 + 입력 잠금)
	UPROPERTY(EditDefaultsOnly, Category = "Animation|HitReact")
	float StunDuration = 0.3f;

public:
	//헌호수정 - 스턴 중 여부 (공격/발사 게이트용)
	bool IsStunned() const { return bIsStunned; }

private:
	//헌호수정 - 피격 시 스턴 적용 / 해제
	void ApplyStun();
	void EndStun();

	bool bIsStunned = false;
	FTimerHandle StunTimerHandle;

protected:

	void HandleHealthChanged(const struct FOnAttributeChangeData& Data);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitReactMontage(UAnimMontage* MontageToPlay);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void InitCamera();
	void InitComponents();

	void ClearDeathRelatedTimers();
	void CleanupBeforeDeathDestroy();

	UFUNCTION()
	void OnItemUsed(FGameplayTag UsedItemTag);

	UFUNCTION()
	void OnConsumableMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool bCameraShaking = false;
	FTimerHandle ShakeTimerHandle;


	float LastHitReactTime = -999.f;

	bool bAimRotationMode = false;
	
	FTimerHandle DeathTimerHandle;

public:
	// 유시환 추가 - 좀비 공격 슬롯 적용
	UFUNCTION(BlueprintPure, Category = "Components|Combat")
	UVGAttackSlotComponent* GetAttackSlotComponent() const { return AttackSlotComponent; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat")
	TObjectPtr<UVGAttackSlotComponent> AttackSlotComponent;
};