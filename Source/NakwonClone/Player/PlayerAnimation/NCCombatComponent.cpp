// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/PlayerAnimation/NCCombatComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

UNCCombatComponent::UNCCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UNCCombatComponent::EquipWeaponCombo(const FNCWeaponComboData& InCombo)
{
    CurrentWeaponCombo = InCombo;
}

UAnimInstance* UNCCombatComponent::GetAnimInstance() const
{
    if (const ACharacter* Char = Cast<ACharacter>(GetOwner()))
    {
        if (const USkeletalMeshComponent* Mesh = Char->GetMesh())
        {
            return Mesh->GetAnimInstance();
        }
    }
    return nullptr;
}

void UNCCombatComponent::MeleeAttack()
{
    UAnimInstance* Anim = GetAnimInstance();
    UAnimMontage* Montage = CurrentWeaponCombo.ComboMontage;
    const TArray<FName>& Sections = CurrentWeaponCombo.ComboSections;
    if (!Anim || !Montage || Sections.Num() == 0) return;

    if (Anim->Montage_IsPlaying(Montage))
    {
        const FName Current = Anim->Montage_GetCurrentSection(Montage);
        const int32 Idx = Sections.IndexOfByKey(Current);
        if (Idx != INDEX_NONE && Idx + 1 < Sections.Num())
        {
            Anim->Montage_SetNextSection(Current, Sections[Idx + 1], Montage);
        }
    }
    else
    {
        Anim->Montage_Play(Montage);
        Anim->Montage_JumpToSection(Sections[0], Montage);
    }
}