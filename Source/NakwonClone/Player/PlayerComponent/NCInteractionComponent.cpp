#include "NCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Item/NCItemActor.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
#include "Animation/AnimInstance.h" //헌호수정

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
		AActor* TargetToInteract = CurrentInteractableTarget;

		INCInteractableInterface::Execute_Interact(TargetToInteract, GetOwner());

		if (LootMontage && OwnerCharacter)
		{
			OwnerCharacter->PlayAnimMontage(LootMontage);
		}

		UpdateInteractableTarget();
		return;
	}

	INCInteractableInterface::Execute_Interact(CurrentInteractableTarget, GetOwner());
}

void UNCInteractionComponent::StopInteraction() //헌호수정 - 사망 시 호출
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateInteractable);
	if (CurrentInteractableTarget)
		SetHighlight(CurrentInteractableTarget, false);
	CurrentInteractableTarget = nullptr;
	bIsLooting = false;
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
