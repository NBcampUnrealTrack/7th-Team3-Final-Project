#include "NCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Item/NCItemActor.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"	
#include "Framwork/PlayerState/NCPlayerState.h"
#include "Player/PlayerController/NCPlayerController.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerComponent/NCEquipmentComponent.h"
#include "Weapon/Gun/NCGunActor.h"
#include "Weapon/Melee/NCMeleePickupActor.h"
#include "Animation/AnimInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

UNCInteractionComponent::UNCInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNCInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle_UpdateInteractable,
			this,
			&UNCInteractionComponent::UpdateInteractableTarget,
			InteractionCheckInterval,
			true
		);
	}
}

void UNCInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateInteractable);
		GetWorld()->GetTimerManager().ClearTimer(PickupUnequipTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UNCInteractionComponent::Interact()
{
	if (!CurrentInteractableTarget)
	{
		return;
	}

	if (CurrentInteractableTarget->IsA<ANCItemActor>())
	{
		if (bIsLooting || bPickupPending)
		{
			return;
		}

		ANCItemActor* Item = Cast<ANCItemActor>(CurrentInteractableTarget);
		if (!IsValid(Item))
		{
			return;
		}

		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (!OwnerCharacter)
		{
			return;
		}

		QueuedPickupTarget = Item;

		bPickupPending = true;

		const bool bPickingUpGun = Item->IsA<ANCGunActor>();
		const bool bPickingUpMelee = Item->IsA<ANCMeleePickupActor>();

		if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
		{
			if (bPickingUpGun)
			{
				if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
				{
					ANCGunActor* GunItem = Cast<ANCGunActor>(Item);
					const FNCGunData* NewGunData = GunItem ? EquipComp->GetGunData(GunItem->GunID) : nullptr;

					if (NewGunData && EquipComp->HasActiveGun() && !EquipComp->IsSwapping())
					{
						if (EquipComp->ActiveSlot == NewGunData->SlotType)
						{
							EquipComp->HideActiveWeaponVisualOnly();
						}
						else
						{
							PendingSelectGunSlotAfterPickup = EquipComp->ActiveSlot;

							if (ANCPlayerController* PC = Cast<ANCPlayerController>(PlayerCharacter->GetController()))
							{
								PC->SetUnArmPending(true);
							}

							EquipComp->OnSwapCompleted.RemoveDynamic(
								this,
								&UNCInteractionComponent::OnGunFullUnequipForPickupFinished);

							EquipComp->OnSwapCompleted.AddDynamic(
								this,
								&UNCInteractionComponent::OnGunFullUnequipForPickupFinished);

							EquipComp->SelectSlot(ENCGunSlot::None);
							return;
						}
					}
				}
			}

			if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
			{
				if (EquipComp->HasActiveGun() && !EquipComp->IsSwapping() && !bPickingUpGun)
				{
					PendingRestoreGunSlot = EquipComp->ActiveSlot;

					float UnequipLength = 0.f;
					if (UNCGunComponent* CurrentWeapon = EquipComp->GetActiveWeapon())
					{
						if (const FNCGunData* Data = CurrentWeapon->GetActiveGunData())
						{
							UnequipLength = CurrentWeapon->PlayUnequipMontage(Data);
						}
					}

					if (UnequipLength > 0.f)
					{
						GetWorld()->GetTimerManager().SetTimer(
							PickupUnequipTimerHandle,
							this,
							&UNCInteractionComponent::OnGunUnequipMontageFinishedForPickup,
							UnequipLength,
							false);
						return;
					}

					EquipComp->HideActiveWeaponVisualOnly();
				}
			}

			if (bPickingUpMelee)
			{
				if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
				{
					if (CombatComp->IsWeaponEquipped() && !CombatComp->IsSwappingWeapon())
					{
						if (AActor* SpawnedWeapon = CombatComp->GetSpawnedWeaponActor())
						{
							SpawnedWeapon->SetActorHiddenInGame(true);
						}
					}
				}
			}

			if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
			{
				if (CombatComp->IsWeaponEquipped() && !CombatComp->IsSwappingWeapon() && !bPickingUpMelee)
				{
					bPendingRestoreMeleeVisual = true;
					PendingRestoreMeleeWeapon = CombatComp->GetEquippedWeapon();

					CombatComp->OnMeleeUnequipCompleted.RemoveDynamic(
						this,
						&UNCInteractionComponent::OnMeleeUnequipForPickupFinished);

					CombatComp->OnMeleeUnequipCompleted.AddDynamic(
						this,
						&UNCInteractionComponent::OnMeleeUnequipForPickupFinished);

					CombatComp->UnEquipWeapon();
					return;
				}
			}
		}

		StartPickup();
		return;
	}

	INCInteractableInterface::Execute_Interact(CurrentInteractableTarget, GetOwner());
}

void UNCInteractionComponent::OnLootMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PickupMontage && Montage != PickupStoreMontage)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (UAnimInstance* AnimInst =
		OwnerCharacter && OwnerCharacter->GetMesh()
		? OwnerCharacter->GetMesh()->GetAnimInstance()
		: nullptr)
	{
		AnimInst->OnMontageEnded.RemoveDynamic(
			this,
			&UNCInteractionComponent::OnLootMontageEndedInternal);
	}

	if (Montage == PickupMontage)
	{
		if (bInterrupted)
		{
			ClearHeldItemMesh();

			if (ANCItemActor* Item = PendingLootTarget.Get())
			{
				if (Item->ItemMesh)
				{
					Item->ItemMesh->SetVisibility(true, true);
					Item->SetActorEnableCollision(true);
				}
			}

			PendingLootTarget = nullptr;
			bIsLooting = false;
			bLootStored = false;
			return;
		}

		if (OwnerCharacter && PickupStoreMontage)
		{
			OwnerCharacter->PlayAnimMontage(PickupStoreMontage);

			if (UAnimInstance* AnimInst =
				OwnerCharacter->GetMesh()
				? OwnerCharacter->GetMesh()->GetAnimInstance()
				: nullptr)
			{
				AnimInst->OnMontageEnded.RemoveDynamic(
					this,
					&UNCInteractionComponent::OnLootMontageEndedInternal);

				AnimInst->OnMontageEnded.AddDynamic(
					this,
					&UNCInteractionComponent::OnLootMontageEndedInternal);
			}

			return;
		}

		ClearHeldItemMesh();
		PendingLootTarget = nullptr;
		bIsLooting = false;
		bLootStored = false;
		return;
	}

	if (Montage == PickupStoreMontage)
	{
		ClearHeldItemMesh();

		if (bInterrupted && !bLootStored)
		{
			if (ANCItemActor* Item = PendingLootTarget.Get())
			{
				if (Item->ItemMesh)
				{
					Item->ItemMesh->SetVisibility(true, true);
					Item->SetActorEnableCollision(true);
				}
			}
		}

		PendingLootTarget = nullptr;
		bIsLooting = false;
		bLootStored = false;
	}
}

void UNCInteractionComponent::AttachLootMeshToHand(ANCItemActor* Item)
{
	ClearHeldItemMesh();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !IsValid(Item) || !Item->ItemMesh)
	{
		return;
	}

	UStaticMesh* Mesh = Item->ItemMesh->GetStaticMesh();
	if (!Mesh)
	{
		return;
	}

	HeldItemMeshComp = NewObject<UStaticMeshComponent>(OwnerCharacter);
	if (!HeldItemMeshComp)
	{
		return;
	}

	HeldItemMeshComp->SetStaticMesh(Mesh);
	HeldItemMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeldItemMeshComp->RegisterComponent();

	HeldItemMeshComp->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		LootHandSocketName
	);

	const FName PickupSocketName = TEXT("PickupHand");

	if (HeldItemMeshComp->DoesSocketExist(PickupSocketName))
	{
		const FTransform PickupSocketLocalTransform =
			HeldItemMeshComp->GetSocketTransform(PickupSocketName, RTS_Component);

		HeldItemMeshComp->SetRelativeLocation(-PickupSocketLocalTransform.GetLocation());
		HeldItemMeshComp->SetRelativeRotation(PickupSocketLocalTransform.GetRotation().Inverse());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupHand socket not found on mesh: %s"), *Mesh->GetName());
	}

	Item->ItemMesh->SetVisibility(false, true);
	Item->SetActorEnableCollision(false);

	if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
	{
		if (ANCGunActor* GunItem = Cast<ANCGunActor>(Item))
		{
			if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
			{
				EquipComp->DropOccupantGunForNewGun(
					GunItem->GunID, Item->GetActorLocation(), Item->GetActorRotation());
			}
		}

		const bool bIsMeleePickup = Item->IsA<ANCMeleePickupActor>();

		if (bIsMeleePickup
			&& !PlayerCharacter->StoredMeleeWeaponID.IsNone()
			&& PlayerCharacter->StoredMeleePickupClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ANCMeleePickupActor* Dropped = GetWorld()->SpawnActor<ANCMeleePickupActor>(
				PlayerCharacter->StoredMeleePickupClass,
				Item->GetActorLocation(),
				PlayerCharacter->StoredMeleePickupRotation,
				Params);

			if (Dropped)
			{
				Dropped->bRespawnEnabled = false;
				Dropped->WeaponID = PlayerCharacter->StoredMeleeWeaponID;
			}

			PlayerCharacter->StoredMeleeWeaponID = NAME_None;
			PlayerCharacter->StoredMeleePickupClass = nullptr;
			PlayerCharacter->OnMeleeStoredChanged.Broadcast();
		}
	}
}

void UNCInteractionComponent::ClearHeldItemMesh()
{
	if (HeldItemMeshComp)
	{
		HeldItemMeshComp->DestroyComponent();
		HeldItemMeshComp = nullptr;
	}
}

void UNCInteractionComponent::StopInteraction()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateInteractable);

	if (CurrentInteractableTarget)
	{
		SetHighlight(CurrentInteractableTarget, false);
	}

	CurrentInteractableTarget = nullptr;
	bIsLooting = false;

	ClearHeldItemMesh();

	if (ANCItemActor* Item = PendingLootTarget.Get())
	{
		if (Item->ItemMesh)
		{
			Item->ItemMesh->SetVisibility(true, true);
			Item->SetActorEnableCollision(true);
		}
	}

	PendingLootTarget = nullptr;
	bLootStored = false;
}

void UNCInteractionComponent::OnLootMontageEnded()
{
	bIsLooting = false;
}

void UNCInteractionComponent::UpdateInteractableTarget()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	FVector SearchLocation = OwnerCharacter->GetActorLocation();

	FCollisionShape RadarSphere = FCollisionShape::MakeSphere(InteractionSearchRadius);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		SearchLocation,
		FQuat::Identity,
		ECC_Visibility,
		RadarSphere,
		QueryParams
	);

	AActor* ClosestTarget = nullptr;
	float MinDistance = InteractionSearchRadius + 1.0f;

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();

			if (HitActor && HitActor->Implements<UNCInteractableInterface>())
			{
				if (HitActor->IsAttachedTo(OwnerCharacter)) continue;

				if (INCInteractableInterface::Execute_CanInteract(HitActor, OwnerCharacter))
				{
					float Distance = FVector::Dist(SearchLocation, HitActor->GetActorLocation());

					if (Distance < MinDistance)
					{
						MinDistance = Distance;
						ClosestTarget = HitActor;
					}
				}
			}
		}
	}

	if (ClosestTarget != CurrentInteractableTarget)
	{
		if (CurrentInteractableTarget)
		{
			SetHighlight(CurrentInteractableTarget, false);
		}

		CurrentInteractableTarget = ClosestTarget;

		if (CurrentInteractableTarget)
		{
			SetHighlight(CurrentInteractableTarget, true);
		}

		OnInteractTargetChanged.Broadcast(CurrentInteractableTarget);
	}
}

void UNCInteractionComponent::SetHighlight(AActor* TargetActor, bool bHighlight)
{
	if (!TargetActor)
	{
		return;
	}

	if (TargetActor->Implements<UNCInteractableInterface>())
	{
		INCInteractableInterface::Execute_ToggleHighlight(TargetActor, bHighlight);
	}
}

void UNCInteractionComponent::AttachPendingLootToHand()
{
	ANCItemActor* Item = PendingLootTarget.Get();
	if (!IsValid(Item))
	{
		return;
	}

	AttachLootMeshToHand(Item);
}

void UNCInteractionComponent::StorePendingLoot()
{
	if (bLootStored)
	{
		return;
	}

	bLootStored = true;

	ClearHeldItemMesh();

	ANCItemActor* Item = PendingLootTarget.Get();
	PendingLootTarget = nullptr;
	QueuedPickupTarget = nullptr;

	if (!IsValid(Item))
	{
		RestorePreviousWeaponAfterPickup();
		return;
	}

	INCInteractableInterface::Execute_Interact(Item, GetOwner());
	UpdateInteractableTarget();

	RestorePreviousWeaponAfterPickup();
}

void UNCInteractionComponent::RestorePreviousWeaponAfterPickup()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	ANCPlayerCharacter* PlayerCharacter = OwnerCharacter ? Cast<ANCPlayerCharacter>(OwnerCharacter) : nullptr;
	if (!PlayerCharacter)
	{
		PendingRestoreGunSlot = ENCGunSlot::None;
		bPendingRestoreMeleeVisual = false;
		PendingSelectGunSlotAfterPickup = ENCGunSlot::None;
		return;
	}

	if (PendingRestoreGunSlot != ENCGunSlot::None)
	{
		if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
		{
			EquipComp->ShowActiveWeaponVisualAgain();
		}
		PendingRestoreGunSlot = ENCGunSlot::None;
	}

	if (PendingSelectGunSlotAfterPickup != ENCGunSlot::None)
	{
		if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
		{
			EquipComp->SelectSlot(PendingSelectGunSlotAfterPickup);
		}
		PendingSelectGunSlotAfterPickup = ENCGunSlot::None;
	}

	if (bPendingRestoreMeleeVisual)
	{
		if (!PendingRestoreMeleeWeapon.WeaponID.IsNone())
		{
			if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
			{
				CombatComp->EquipWeapon(PendingRestoreMeleeWeapon);
			}
		}
		bPendingRestoreMeleeVisual = false;
		PendingRestoreMeleeWeapon = FNCWeaponInstance();
	}
}

void UNCInteractionComponent::OnGunUnequipMontageFinishedForPickup()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
		{
			if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
			{
				EquipComp->HideActiveWeaponVisualOnly();
			}
		}
	}

	StartPickup();
}

void UNCInteractionComponent::OnGunFullUnequipForPickupFinished(ENCGunSlot FinishedSlot)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
		{
			if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
			{
				EquipComp->OnSwapCompleted.RemoveDynamic(
					this,
					&UNCInteractionComponent::OnGunFullUnequipForPickupFinished);
			}
		}
	}

	StartPickup();
}

void UNCInteractionComponent::OnMeleeUnequipForPickupFinished()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
		{
			if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
			{
				CombatComp->OnMeleeUnequipCompleted.RemoveDynamic(
					this,
					&UNCInteractionComponent::OnMeleeUnequipForPickupFinished);
			}
		}
	}

	StartPickup();
}

void UNCInteractionComponent::StartPickup()
{
	bPickupPending = false;

	if (bIsLooting)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	ANCItemActor* Item = QueuedPickupTarget.Get();

	if (!OwnerCharacter || !IsValid(Item))
	{
		QueuedPickupTarget = nullptr;
		return;
	}

	if (!PickupMontage)
	{
		INCInteractableInterface::Execute_Interact(Item, GetOwner());
		UpdateInteractableTarget();
		QueuedPickupTarget = nullptr;
		return;
	}

	bIsLooting = true;
	bLootStored = false;
	PendingLootTarget = Item;

	OwnerCharacter->PlayAnimMontage(PickupMontage);

	if (UAnimInstance* AnimInst =
		OwnerCharacter->GetMesh()
		? OwnerCharacter->GetMesh()->GetAnimInstance()
		: nullptr)
	{
		AnimInst->OnMontageEnded.RemoveDynamic(
			this,
			&UNCInteractionComponent::OnLootMontageEndedInternal);

		AnimInst->OnMontageEnded.AddDynamic(
			this,
			&UNCInteractionComponent::OnLootMontageEndedInternal);
	}
}

