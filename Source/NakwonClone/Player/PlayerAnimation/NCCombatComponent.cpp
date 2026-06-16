// Fill out your copyright notice in the Description page of Project Settings.

#include "NCCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Framwork/GameInstacne/NCGameInstance.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"

UNCCombatComponent::UNCCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UNCCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ANCBaseCharacter>(GetOwner());

	if (OwnerCharacter)
	{
		ASC = OwnerCharacter->GetAbilitySystemComponent();
	}
}

void UNCCombatComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNCCombatComponent, EquippedWeapon);
}

void UNCCombatComponent::EquipWeaponCombo(const FNCWeaponComboData& InCombo)
{
	CurrentWeaponCombo = InCombo;
}

UAnimInstance* UNCCombatComponent::GetAnimInstance() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (const USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			return Mesh->GetAnimInstance();
		}
	}

	return nullptr;
}

void UNCCombatComponent::MeleeAttack()
{
	if (!CanAttack())
	{
		return;
	}

	UAnimInstance* Anim = GetAnimInstance();
	UAnimMontage* Montage = CurrentWeaponCombo.ComboMontage;
	const TArray<FName>& Sections = CurrentWeaponCombo.ComboSections;

	if (!Anim || !Montage || Sections.Num() == 0)
	{
		return;
	}

	if (Anim->Montage_IsPlaying(Montage))
	{
		const FName CurrentSection = Anim->Montage_GetCurrentSection(Montage);
		const int32 CurrentIndex = Sections.IndexOfByKey(CurrentSection);

		if (CurrentIndex != INDEX_NONE && CurrentIndex + 1 < Sections.Num())
		{
			Anim->Montage_SetNextSection(
				CurrentSection,
				Sections[CurrentIndex + 1],
				Montage
			);
		}
	}
	else
	{
		Anim->Montage_Play(Montage);
		Anim->Montage_JumpToSection(Sections[0], Montage);
	}
}

void UNCCombatComponent::EquipWeapon(FNCWeaponInstance WeaponInstance)
{
	if (!WeaponInstance.IsValid())
	{
		return;
	}

	Server_EquipWeapon(WeaponInstance);
}

void UNCCombatComponent::UnEquipWeapon()
{
	if (!bIsEquipped)
	{
		return;
	}

	Server_UnEquipWeapon();
}

void UNCCombatComponent::ReduceDurability(float Amount)
{
	if (!bIsEquipped)
	{
		return;
	}

	Server_ReduceDurability(Amount);
}

void UNCCombatComponent::Server_EquipWeapon_Implementation(
	FNCWeaponInstance WeaponInstance)
{
	Internal_EquipWeapon(WeaponInstance);
}

void UNCCombatComponent::Server_UnEquipWeapon_Implementation()
{
	Internal_UnEquipWeapon();
}

void UNCCombatComponent::Server_ReduceDurability_Implementation(float Amount)
{
	EquippedWeapon.CurrentDurability -= Amount;
	EquippedWeapon.CurrentDurability =
		FMath::Max(0.f, EquippedWeapon.CurrentDurability);

	if (EquippedWeapon.CurrentDurability <= 0.f)
	{
		EquippedWeapon.bIsBroken = true;

		if (ASC)
		{
			ASC->AddLooseGameplayTag(NCWeapon::State_Broken);
		}
	}
}

void UNCCombatComponent::Internal_EquipWeapon(FNCWeaponInstance WeaponInstance)
{
	EquippedWeapon = WeaponInstance;
	bIsEquipped = true;

	if (ASC)
	{
		FNCWeaponData* Data = GetEquippedWeaponData();

		if (Data)
		{
			ASC->AddLooseGameplayTag(Data->WeaponTypeTag);
			ASC->AddLooseGameplayTag(Data->WeightTag);
			ASC->AddLooseGameplayTag(NCWeapon::State_Equipped);

			// 콤보 데이터 설정
			FNCWeaponComboData ComboData;
			ComboData.ComboMontage = Data->AttackMontage.LoadSynchronous();
			ComboData.ComboSections = { TEXT("Attack1"), TEXT("Attack2"), TEXT("Attack3"), TEXT("Attack4") };
			EquipWeaponCombo(ComboData);

			// 무기 액터 스폰 후 손에 부착
			if (!Data->WeaponActorClass.IsNull())
			{
				UClass* ActorClass = Data->WeaponActorClass.LoadSynchronous();
				if (ActorClass && OwnerCharacter)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = OwnerCharacter;
					SpawnParams.Instigator = OwnerCharacter;

					SpawnedWeaponActor = GetWorld()->SpawnActor<AActor>(
						ActorClass, FTransform::Identity, SpawnParams);

					if (SpawnedWeaponActor)
					{
						SpawnedWeaponActor->AttachToComponent(
							OwnerCharacter->GetMesh(),
							FAttachmentTransformRules::SnapToTargetNotIncludingScale,
							Data->AttachSocketName);
					}
				}
			}
		}
	}

	OnWeaponChanged.Broadcast(EquippedWeapon);
}

void UNCCombatComponent::Internal_UnEquipWeapon()
{
	if (ASC)
	{
		FNCWeaponData* Data = GetEquippedWeaponData();

		if (Data)
		{
			ASC->RemoveLooseGameplayTag(Data->WeaponTypeTag);
			ASC->RemoveLooseGameplayTag(Data->WeightTag);
			ASC->RemoveLooseGameplayTag(NCWeapon::State_Equipped);
			// 헌호수정 - 파손 상태 태그도 제거 (다음 무기에 영향 방지)
			if (EquippedWeapon.bIsBroken)
				ASC->RemoveLooseGameplayTag(NCWeapon::State_Broken);
		}
	}

	// 무기 액터 제거
	if (SpawnedWeaponActor)
	{
		SpawnedWeaponActor->Destroy();
		SpawnedWeaponActor = nullptr;
	}

	EquippedWeapon = FNCWeaponInstance();
	bIsEquipped = false;

	OnWeaponChanged.Broadcast(FNCWeaponInstance{});
}

void UNCCombatComponent::OnRep_EquippedWeapon()
{
	bIsEquipped = EquippedWeapon.IsValid();

	if (EquippedWeapon.bIsBroken && ASC)
	{
		ASC->AddLooseGameplayTag(NCWeapon::State_Broken);
	}

	OnWeaponChanged.Broadcast(EquippedWeapon);
}

FGameplayTag UNCCombatComponent::GetEquippedWeaponTypeTag() const
{
	const FNCWeaponData* WeaponData = GetEquippedWeaponData();
	if (!WeaponData)
	{
		return NCWeapon::Type_Unarmed;
	}
	return WeaponData->WeaponTypeTag;
}

FNCWeaponData* UNCCombatComponent::GetEquippedWeaponData() const
{
	if (!bIsEquipped || !OwnerCharacter)
	{
		return nullptr;
	}

	UNCGameInstance* GI = Cast<UNCGameInstance>(
		OwnerCharacter->GetGameInstance()
	);

	if (!GI)
	{
		return nullptr;
	}

	return GI->GetWeaponData(EquippedWeapon.WeaponID);
}

bool UNCCombatComponent::CanAttack() const
{
	if (!bIsEquipped)
	{
		return false;
	}

	if (EquippedWeapon.bIsBroken)
	{
		return false;
	}

	if (!ASC)
	{
		return false;
	}

	if (ASC->HasMatchingGameplayTag(NCWeapon::Action_Swapping))
	{
		return false;
	}

	return true;
}