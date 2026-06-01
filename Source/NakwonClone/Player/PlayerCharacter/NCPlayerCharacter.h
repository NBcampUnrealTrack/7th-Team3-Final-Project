#pragma once

#include "CoreMinimal.h"
#include "NCBaseCharacter.h"
#include "NCPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNCPlayerInventoryComponent;

UCLASS()
class NAKWONCLONE_API ANCPlayerCharacter : public ANCBaseCharacter
{
	GENERATED_BODY()

public:
	ANCPlayerCharacter();
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UNCPlayerInventoryComponent* GetInventoryComponent() const { return PlayerInventory; }

protected:
	virtual void BeginPlay() override;

public:
	void StartSprint();
	void StopSprint();
	void ToggleWalk();
	void ToggleCrouch();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Inventory")
	TObjectPtr<UNCPlayerInventoryComponent> PlayerInventory;
	
public:
	//무기 장착
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(TSubclassOf<ANCWeaponBase> WeaponClass);

	//무기 해제
	UFUNCTION(BlueprintCallable)
	void UnEquipWeapon();

protected:
	//현재 장착된 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ANCWeaponBase> CurrentWeapon;
};