#include "NCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Item/NCItemActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Player/PlayerController/NCPlayerController.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
#include "Animation/AnimInstance.h" //헌호수정
#include "Components/StaticMeshComponent.h" //헌호수정
#include "Components/WidgetComponent.h" //헌호수정

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

		if (LootMontage && OwnerCharacter)
		{
			bIsLooting = true;
			bLootStored = false;
			PendingLootTarget = Item;

			OwnerCharacter->PlayAnimMontage(LootMontage);

			if (OwnerCharacter->GetCharacterMovement())
			{
				OwnerCharacter->GetCharacterMovement()->DisableMovement();
			}

			if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
			{
				PC->SetIgnoreMoveInput(true);
				PC->SetIgnoreLookInput(true);
			}

			if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr)
			{
				AnimInst->OnMontageEnded.RemoveDynamic(this, &UNCInteractionComponent::OnLootMontageEndedInternal);
				AnimInst->OnMontageEnded.AddDynamic(this, &UNCInteractionComponent::OnLootMontageEndedInternal);
			}

			return;
		}

		INCInteractableInterface::Execute_Interact(Item, GetOwner());
		UpdateInteractableTarget();
		return;
	}

	INCInteractableInterface::Execute_Interact(CurrentInteractableTarget, GetOwner());
}

//헌호수정 - 줍기 몽타주 종료 시: 중단이면 취소, 정상 종료면 실제 획득
void UNCInteractionComponent::OnLootMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != LootMontage)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (UAnimInstance* AnimInst = OwnerCharacter && OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInst->OnMontageEnded.RemoveDynamic(this, &UNCInteractionComponent::OnLootMontageEndedInternal);
	}

	if (OwnerCharacter)
	{
		if (OwnerCharacter->GetCharacterMovement())
		{
			OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
		}
	}

	ClearHeldItemMesh();

	ANCItemActor* Item = PendingLootTarget.Get();

	if (bInterrupted && !bLootStored)
	{
		if (IsValid(Item) && Item->ItemMesh)
		{
			Item->ItemMesh->SetVisibility(true);
		}
	}

	PendingLootTarget = nullptr;
	bIsLooting = false;
	bLootStored = false;
}

//헌호수정 - 손 소켓에 임시 시각용 메시 부착 + 바닥 아이템 숨김
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

	// 1차로 캐릭터 손 소켓에 부착
	HeldItemMeshComp->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		LootHandSocketName
	);

	// StaticMesh 안의 PickupHand 소켓이 hand_ItemSocket에 오도록 역보정
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

//헌호수정 - 손에 붙인 임시 메시 제거
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

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (OwnerCharacter->GetCharacterMovement())
		{
			OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
		}
	}

	ClearHeldItemMesh();

	if (ANCItemActor* Item = PendingLootTarget.Get())
	{
		if (Item->ItemMesh)
		{
			Item->ItemMesh->SetVisibility(true);
		}
	}

	PendingLootTarget = nullptr;
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
				// 헌호수정 - 캐릭터에 부착된 액터(장착된 무기 등)는 상호작용 대상 제외
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