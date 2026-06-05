// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NCCombatComponent.generated.h"

class UAnimMontage;
class UAnimInstance;

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

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void MeleeAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EquipWeaponCombo(const FNCWeaponComboData& InCombo);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FNCWeaponComboData CurrentWeaponCombo;

    UAnimInstance* GetAnimInstance() const;
};