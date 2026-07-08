// Fill out your copyright notice in the Description page of Project Settings.

#include "NCCombatComponent.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Framwork/GameInstacne/NCGameInstance.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h" //헌호수정

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

	if (!CurrentWeaponCombo.ComboMontage)
	{
		if (OwnerCharacter)
		{
			if (UNCGameInstance* GI = Cast<UNCGameInstance>(OwnerCharacter->GetGameInstance()))
			{
				if (FNCWeaponData* UnarmedData = GI->GetWeaponData(TEXT("Unarmed")))
				{
					FNCWeaponComboData ComboData;
					ComboData.ComboMontage = UnarmedData->AttackMontage.LoadSynchronous();
					ComboData.ComboSections = UnarmedData->AttackSections.Num() > 0
						? UnarmedData->AttackSections
						: TArray<FName>{ TEXT("Attack1") };

					EquipWeaponCombo(ComboData);
				}
			}
		}
	}

	UAnimInstance* Anim = GetAnimInstance();
	UAnimMontage* Montage = CurrentWeaponCombo.ComboMontage;
	const TArray<FName>& Sections = CurrentWeaponCombo.ComboSections;

	if (!Anim || !Montage || Sections.Num() == 0)
	{
		return;
	}

	LastPlayedAttackMontage = Montage;

	// 공격 몽타주가 재생 중이면 새 공격 시작이 아니라 콤보 입력 예약만 한다.
	if (Anim->Montage_IsPlaying(Montage))
	{
		bComboInputBuffered = true;
		return;
	}

	ResetComboState();

	bIsMeleeComboPlaying = true;
	CurrentComboIndex = 0;

	Anim->OnMontageEnded.RemoveDynamic(
		this,
		&UNCCombatComponent::OnMeleeAttackMontageEnded);

	Anim->OnMontageEnded.AddDynamic(
		this,
		&UNCCombatComponent::OnMeleeAttackMontageEnded);

	Anim->Montage_Play(Montage);
	Anim->Montage_JumpToSection(Sections[CurrentComboIndex], Montage);
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
	if (!WeaponInstance.IsValid())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(MeleeEquipTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(MeleeUnequipTimerHandle);

	++SwapGeneration;

	// 기존 근접무기 액터가 남아있으면 먼저 제거
	if (SpawnedWeaponActor)
	{
		SpawnedWeaponActor->Destroy();
		SpawnedWeaponActor = nullptr;
	}

	// 기존 무기 태그 정리
	if (ASC)
	{
		if (FNCWeaponData* OldData = GetEquippedWeaponData())
		{
			ASC->RemoveLooseGameplayTag(OldData->WeaponTypeTag);
			ASC->RemoveLooseGameplayTag(OldData->WeightTag);
		}

		ASC->RemoveLooseGameplayTag(NCWeapon::State_Equipped);
		ASC->RemoveLooseGameplayTag(NCWeapon::State_Broken);

		// 총기/ADS 쪽 잔여 태그 정리
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Aiming);
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Firing);
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Reloading);
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Swapping);
	}

	EquippedWeapon = WeaponInstance;
	bIsEquipped = true;
	bIsSwappingWeapon = true;

	if (ASC)
	{
		FNCWeaponData* Data = GetEquippedWeaponData();

		if (Data)
		{
			ASC->AddLooseGameplayTag(Data->WeaponTypeTag);
			ASC->AddLooseGameplayTag(Data->WeightTag);
			ASC->AddLooseGameplayTag(NCWeapon::State_Equipped);
			ASC->AddLooseGameplayTag(NCWeapon::Action_Swapping);

			FNCWeaponComboData ComboData;
			ComboData.ComboMontage = Data->AttackMontage.LoadSynchronous();
			ComboData.ComboSections = Data->AttackSections.Num() > 0
				? Data->AttackSections
				: TArray<FName>{ TEXT("Attack1") };

			EquipWeaponCombo(ComboData);

			PlayEquipMontage();
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		MeleeEquipTimerHandle,
		FTimerDelegate::CreateUObject(this, &UNCCombatComponent::FinishEquipWeapon, SwapGeneration),
		MeleeEquipAttachDelay,
		false
	);

	OnWeaponChanged.Broadcast(EquippedWeapon);
}

void UNCCombatComponent::Internal_UnEquipWeapon()
{
	if (!bIsEquipped)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(MeleeEquipTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(MeleeUnequipTimerHandle);

	++SwapGeneration;

	bIsSwappingWeapon = true;

	if (ASC)
	{
		ASC->AddLooseGameplayTag(NCWeapon::Action_Swapping);
	}

	PlayUnequipMontage();

	GetWorld()->GetTimerManager().SetTimer(
		MeleeUnequipTimerHandle,
		FTimerDelegate::CreateUObject(this, &UNCCombatComponent::FinishUnEquipWeapon, SwapGeneration),
		MeleeUnequipDetachDelay,
		false
	);
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
	if (!ASC)
	{
		return false;
	}

	if (ASC->HasMatchingGameplayTag(NCWeapon::Action_Swapping))
	{
		return false;
	}

	//헌호수정 - 스턴 중 공격 불가
	if (ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(GetOwner()))
	{
		if (Player->IsStunned())
		{
			return false;
		}
	}

	if (bIsEquipped && EquippedWeapon.bIsBroken)
	{
		return false;
	}

	return true;
}

bool UNCCombatComponent::IsGunWeapon() const
{
	const FNCWeaponData* Data = GetEquippedWeaponData();

	if (!Data)
	{
		return false;
	}

	return !Data->FireMontage.IsNull();
}

void UNCCombatComponent::GunAttack()
{
	if (!CanAttack())
	{
		return;
	}

	FNCWeaponData* Data = GetEquippedWeaponData();

	if (!Data || Data->FireMontage.IsNull())
	{
		return;
	}

	UAnimInstance* Anim = GetAnimInstance();

	if (!Anim)
	{
		return;
	}

	UAnimMontage* Montage = Data->FireMontage.LoadSynchronous();

	if (!Montage)
	{
		return;
	}

	LastPlayedAttackMontage = Montage;

	if (ASC)
	{
		ASC->AddLooseGameplayTag(NCWeapon::Action_Firing);
	}

	Anim->Montage_Play(Montage);
}

void UNCCombatComponent::PlayReloadMontage()
{
	FNCWeaponData* Data = GetEquippedWeaponData();

	if (!Data || Data->ReloadMontage.IsNull())
	{
		return;
	}

	UAnimInstance* Anim = GetAnimInstance();

	if (!Anim)
	{
		return;
	}

	UAnimMontage* Montage = Data->ReloadMontage.LoadSynchronous();

	if (!Montage)
	{
		return;
	}

	if (ASC)
	{
		ASC->AddLooseGameplayTag(NCWeapon::Action_Reloading);
	}

	Anim->Montage_Play(Montage);
}

void UNCCombatComponent::PlayEquipMontage()
{
	FNCWeaponData* Data = GetEquippedWeaponData();

	if (!Data || Data->EquipMontage.IsNull())
	{
		return;
	}

	UAnimInstance* Anim = GetAnimInstance();

	if (!Anim)
	{
		return;
	}

	UAnimMontage* Montage = Data->EquipMontage.LoadSynchronous();

	if (!Montage)
	{
		return;
	}

	Anim->Montage_Play(Montage);
}

void UNCCombatComponent::FinishEquipWeapon(uint32 Generation)
{
	if (Generation != SwapGeneration)
	{
		return;
	}

	FNCWeaponData* Data = GetEquippedWeaponData();

	if (!Data || !OwnerCharacter)
	{
		bIsSwappingWeapon = false;

		if (ASC)
		{
			ASC->RemoveLooseGameplayTag(NCWeapon::Action_Swapping);
		}

		return;
	}

	if (!Data->WeaponActorClass.IsNull())
	{
		UClass* ActorClass = Data->WeaponActorClass.LoadSynchronous();

		if (ActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerCharacter;
			SpawnParams.Instigator = OwnerCharacter;

			SpawnedWeaponActor = GetWorld()->SpawnActor<AActor>(
				ActorClass,
				FTransform::Identity,
				SpawnParams
			);

			if (SpawnedWeaponActor)
			{
				SpawnedWeaponActor->AttachToComponent(
					OwnerCharacter->GetMesh(),
					FAttachmentTransformRules::SnapToTargetIncludingScale,
					Data->AttachSocketName
				);
			}
		}
	}

	bIsSwappingWeapon = false;

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Swapping);
	}
}

void UNCCombatComponent::FinishUnEquipWeapon(uint32 Generation)
{
	if (Generation != SwapGeneration)
	{
		return;
	}

	if (ASC)
	{
		FNCWeaponData* Data = GetEquippedWeaponData();

		if (Data)
		{
			ASC->RemoveLooseGameplayTag(Data->WeaponTypeTag);
			ASC->RemoveLooseGameplayTag(Data->WeightTag);
			ASC->RemoveLooseGameplayTag(NCWeapon::State_Equipped);

			if (EquippedWeapon.bIsBroken)
			{
				ASC->RemoveLooseGameplayTag(NCWeapon::State_Broken);
			}
		}

		ASC->RemoveLooseGameplayTag(NCWeapon::Action_Swapping);
	}

	if (SpawnedWeaponActor)
	{
		SpawnedWeaponActor->Destroy();
		SpawnedWeaponActor = nullptr;
	}

	EquippedWeapon = FNCWeaponInstance();
	bIsEquipped = false;
	bIsSwappingWeapon = false;

	OnWeaponChanged.Broadcast(FNCWeaponInstance{});
	OnMeleeUnequipCompleted.Broadcast();
}

void UNCCombatComponent::PlayUnequipMontage()
{
	FNCWeaponData* Data = GetEquippedWeaponData();

	if (!Data || Data->UnequipMontage.IsNull())
	{
		return;
	}

	UAnimInstance* Anim = GetAnimInstance();

	if (!Anim)
	{
		return;
	}

	UAnimMontage* Montage = Data->UnequipMontage.LoadSynchronous();

	if (!Montage)
	{
		return;
	}

	Anim->Montage_Play(Montage);
}

void UNCCombatComponent::SetSpawnedWeaponVisible(bool bVisible)
{
	if (!SpawnedWeaponActor)
	{
		return;
	}

	SpawnedWeaponActor->SetActorHiddenInGame(!bVisible);
	SpawnedWeaponActor->SetActorEnableCollision(bVisible);
}

void UNCCombatComponent::OpenComboInput()
{
	if (!bIsMeleeComboPlaying)
	{
		return;
	}

	bCanStartNextAttack = true;
}

void UNCCombatComponent::HandleComboBranch()
{
	if (!bIsMeleeComboPlaying)
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

	if (!Anim->Montage_IsPlaying(Montage))
	{
		return;
	}

	bCanStartNextAttack = false;

	// 입력 예약이 없으면 현재 공격 끝까지 재생
	if (!bComboInputBuffered)
	{
		return;
	}

	bComboInputBuffered = false;

	int32 NextComboIndex = CurrentComboIndex + 1;

	// 3타 무기면 Attack3 다음 Attack1
	// 4타 무기면 Attack4 다음 Attack1
	if (!Sections.IsValidIndex(NextComboIndex))
	{
		NextComboIndex = 0;
	}

	if (!Sections.IsValidIndex(NextComboIndex))
	{
		return;
	}

	if (!Montage->IsValidSectionName(Sections[NextComboIndex]))
	{
		return;
	}

	CurrentComboIndex = NextComboIndex;

	// 루프 점프 이후에도 콤보 상태 유지
	bIsMeleeComboPlaying = true;
	bCanStartNextAttack = false;
	bComboInputBuffered = false;

	Anim->Montage_JumpToSection(Sections[CurrentComboIndex], Montage);
}

void UNCCombatComponent::OnMeleeAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CurrentWeaponCombo.ComboMontage)
	{
		return;
	}

	UAnimInstance* Anim = GetAnimInstance();

	// 핵심:
	// Attack3 → Attack1, Attack4 → Attack1처럼 섹션 루프 점프 직후
	// MontageEnded가 늦게 들어오는 경우가 있음.
	// 그런데 실제로 몽타주가 아직 재생 중이면 콤보 상태를 리셋하면 안 됨.
	if (Anim && Anim->Montage_IsPlaying(Montage))
	{
		return;
	}

	if (Anim)
	{
		Anim->OnMontageEnded.RemoveDynamic(
			this,
			&UNCCombatComponent::OnMeleeAttackMontageEnded);
	}

	ResetComboState();
}

void UNCCombatComponent::ResetComboState()
{
	bCanStartNextAttack = false;
	bComboInputBuffered = false;
	bIsMeleeComboPlaying = false;
	CurrentComboIndex = 0;
}

bool UNCCombatComponent::IsMeleeAttackMontagePlaying() const
{
	UAnimInstance* Anim = GetAnimInstance();
	UAnimMontage* Montage = CurrentWeaponCombo.ComboMontage;

	if (!Anim || !Montage)
	{
		return false;
	}

	return Anim->Montage_IsPlaying(Montage);
}