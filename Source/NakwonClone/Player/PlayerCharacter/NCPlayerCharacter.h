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

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void OnUseItemMontageEnded();
	
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

protected:
	UFUNCTION(Server, Reliable)
	void Server_SetGait(FGameplayTag NewGaitTag);

	UFUNCTION(Server, Reliable)
	void Server_SetStance(FGameplayTag NewStanceTag);

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
	
	//H Movement 카테고리
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 200.f;
	//H
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 600.f;

	//H Input 카테고리 (Enhanced Input 쓴다면)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> SprintAction;

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

	//H 입력 콜백 함수
	void OnSprintStarted(const struct FInputActionValue& Value);
	void OnSprintStopped(const struct FInputActionValue& Value);

	//하상빈 추가
	UFUNCTION()
	void OnItemUsed(FGameplayTag UsedItemTag);
};