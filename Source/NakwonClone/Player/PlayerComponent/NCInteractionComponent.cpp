#include "NCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Item/NCItemActor.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
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

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (CurrentInteractableTarget->IsA<ANCItemActor>())
	{
		if (bIsLooting)
		{
			return;
		}

		ANCItemActor* Item = Cast<ANCItemActor>(CurrentInteractableTarget);

		if (PickupMontage && OwnerCharacter)
		{
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

			return;
		}

		INCInteractableInterface::Execute_Interact(Item, GetOwner());
		UpdateInteractableTarget();
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

	// 첫 번째 몽타주: 숙이기 + 잡기 + 들어올리기
	if (Montage == PickupMontage)
	{
		if (bInterrupted)
		{
			ClearHeldItemMesh();

			if (ANCItemActor* Item = PendingLootTarget.Get())
			{
				if (Item->ItemMesh)
				{
					Item->ItemMesh->SetVisibility(true);
				}
			}

			PendingLootTarget = nullptr;
			bIsLooting = false;
			bLootStored = false;
			return;
		}

		// 첫 번째 몽타주가 정상 종료되면 두 번째 몽타주 재생
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

		// 두 번째 몽타주가 없으면 바로 정리
		ClearHeldItemMesh();
		PendingLootTarget = nullptr;
		bIsLooting = false;
		bLootStored = false;
		return;
	}

	// 두 번째 몽타주: 등에 넣기
	if (Montage == PickupStoreMontage)
	{
		ClearHeldItemMesh();

		if (bInterrupted && !bLootStored)
		{
			if (ANCItemActor* Item = PendingLootTarget.Get())
			{
				if (Item->ItemMesh)
				{
					Item->ItemMesh->SetVisibility(true);
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

	Item->ItemMesh->SetVisibility(false);
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
			Item->ItemMesh->SetVisibility(true);
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

	if (!IsValid(Item))
	{
		return;
	}

	INCInteractableInterface::Execute_Interact(Item, GetOwner());
	UpdateInteractableTarget();
}