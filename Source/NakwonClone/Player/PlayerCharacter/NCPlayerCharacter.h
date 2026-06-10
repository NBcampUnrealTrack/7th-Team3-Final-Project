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
	void OnDead();

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
	
private:
	void InitCamera();
	void InitComponents();

};