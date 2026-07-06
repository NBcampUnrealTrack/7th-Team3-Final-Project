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

void UNCInteractionComponent::Interact()
{
	if (!CurrentInteractableTarget)
	{
		return;
	}

	if (CurrentInteractableTarget->IsA<ANCItemActor>())
	{
		if (bIsLooting)
		{
			return;
		}

		ANCItemActor* Item = Cast<ANCItemActor>(CurrentInteractableTarget);
		if (!IsValid(Item))
		{
			return;
		}

		QueuedPickupTarget = Item;

		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (!OwnerCharacter)
		{
			return;
		}

		if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
		{
			// 1. 총기 장착 중이면 총기 Unequip 후 Pickup
			if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
			{
				if (EquipComp->HasActiveGun() && !EquipComp->IsSwapping())
				{
					// 핵심 수정:
					// 총기 Unequip 완료 후 저장된 근접무기가 자동 장착되는 것을 막는다.
					// 이게 없으면 라이플을 내리는 순간 기존 도끼/카타나가 손에 장착되고,
					// Pickup Attach 시 새 무기까지 붙어서 무기가 2개 보인다.
					if (ANCPlayerController* PC = Cast<ANCPlayerController>(PlayerCharacter->GetController()))
					{
						PC->SetUnArmPending(true);
					}

					EquipComp->OnSwapCompleted.RemoveDynamic(
						this,
						&UNCInteractionComponent::OnGunUnequipForPickupFinished);

					EquipComp->OnSwapCompleted.AddDynamic(
						this,
						&UNCInteractionComponent::OnGunUnequipForPickupFinished);

					EquipComp->SelectSlot(ENCGunSlot::None);
					return;
				}
			}

			// 2. 근접무기 장착 중이면 근접 Unequip 후 Pickup
			if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
			{
				if (CombatComp->IsWeaponEquipped())
				{
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
		return;
	}

	INCInteractableInterface::Execute_Interact(Item, GetOwner());
	UpdateInteractableTarget();
}

void UNCInteractionComponent::StartPickup()
{
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

	// ─────────────────────────────────────
	// 핵심 수정
	// 무기 Pickup 시작 전에 기존 Stored 근접무기를 먼저 Drop + 시각 제거
	// ─────────────────────────────────────
	if (Item->ItemTypeTag.MatchesTag(NCItemTag::Weapon))
	{
		if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
		{
			const bool bHadStoredMelee = !PlayerCharacter->StoredMeleeWeaponID.IsNone();

			if (bHadStoredMelee)
			{
				// 서버에는 실제 바닥 DropActor 스폰 요청
				if (ANCPlayerState* PlayerState = OwnerCharacter->GetPlayerState<ANCPlayerState>())
				{
					if (UNCPlayerInventoryComponent* InventoryComp =
						PlayerState->FindComponentByClass<UNCPlayerInventoryComponent>())
					{
						InventoryComp->DropStoredMeleeForPickupReplace();
					}
				}

				// 로컬 시각 메쉬 즉시 제거
				// 이걸 안 하면 PickupMontage 중 기존 근접무기 + 새 무기가 같이 보임
				PlayerCharacter->StoredMeleeWeaponID = NAME_None;
				PlayerCharacter->StoredMeleePickupClass = nullptr;
				PlayerCharacter->OnMeleeStoredChanged.Broadcast();
			}
		}
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

void UNCInteractionComponent::OnMeleeUnequipForPickupFinished()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
	{
		if (UNCCombatComponent* CombatComp = PlayerCharacter->GetCombatComponent())
		{
			CombatComp->OnMeleeUnequipCompleted.RemoveDynamic(
				this,
				&UNCInteractionComponent::OnMeleeUnequipForPickupFinished);
		}
	}

	StartPickup();
}

void UNCInteractionComponent::OnGunUnequipForPickupFinished(ENCGunSlot FinishedSlot)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(OwnerCharacter))
	{
		if (UNCEquipmentComponent* EquipComp = PlayerCharacter->GetEquipmentComponent())
		{
			EquipComp->OnSwapCompleted.RemoveDynamic(
				this,
				&UNCInteractionComponent::OnGunUnequipForPickupFinished);
		}
	}

	StartPickup();
}