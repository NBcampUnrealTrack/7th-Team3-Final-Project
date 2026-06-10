// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NCCombatComponent.generated.h"

class UAnimMontage;
class UAnimInstance;
class ANCBaseCharacter;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnWeaponChanged, const FNCWeaponInstance&, NewWeapon);

USTRUCT(BlueprintType)
struct FNCWeaponComboData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UAnimMontage> ComboMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FName> ComboSections;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCCombatComponent();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void MeleeAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeaponCombo(const FNCWeaponComboData& InCombo);

	void EquipWeapon(FNCWeaponInstance WeaponInstance);
	void UnEquipWeapon();

	FNCWeaponInstance GetEquippedWeapon() const { return EquippedWeapon; }

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnWeaponChanged OnWeaponChanged;

	UFUNCTION(BlueprintPure, Category = "Combat|Weapon")
	AActor* GetSpawnedWeaponActor() const
	{
		return SpawnedWeaponActor;
	}

	FNCWeaponData* GetEquippedWeaponData() const;

	bool CanAttack() const;

	UFUNCTION(BlueprintPure, Category = "Combat|Weapon")
	FGameplayTag GetEquippedWeaponTypeTag() const;

	UFUNCTION(BlueprintPure, Category = "Combat|Weapon")
	bool IsWeaponEquipped() const { return bIsEquipped; }

	void ReduceDurability(float Amount);

	UAnimMontage* GetCurrentComboMontage() const { return CurrentWeaponCombo.ComboMontage; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FNCWeaponComboData CurrentWeaponCombo;

	UAnimInstance* GetAnimInstance() const;

private:
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(FNCWeaponInstance WeaponInstance);

	UFUNCTION(Server, Reliable)
	void Server_UnEquipWeapon();

	UFUNCTION(Server, Reliable)
	void Server_ReduceDurability(float Amount);

	void Internal_EquipWeapon(FNCWeaponInstance WeaponInstance);
	void Internal_UnEquipWeapon();

	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	UPROPERTY()
	TObjectPtr<ANCBaseCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	FNCWeaponInstance EquippedWeapon;

	// 현재 손에 들고 있는 무기 액터
	UPROPERTY()
	TObjectPtr<AActor> SpawnedWeaponActor;

	bool bIsEquipped = false;
};