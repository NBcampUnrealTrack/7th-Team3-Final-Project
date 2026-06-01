#include "NCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NakwonClone/Common/NCInteractableInterface.h"

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
	if (CurrentInteractableTarget)
	{
		INCInteractableInterface::Execute_Interact(CurrentInteractableTarget, GetOwner());
	}
}

void UNCInteractionComponent::UpdateInteractableTarget()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}
	
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}
	
	FVector TraceStart = PC->PlayerCameraManager->GetCameraLocation();
	FVector TraceForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	FVector TraceEnd = TraceStart + (TraceForward * InteractionTraceDistance);
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	FCollisionShape Sphere = FCollisionShape::MakeSphere(15.0f);
	
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, 
        TraceStart, 
        TraceEnd, 
        FQuat::Identity, 
        ECC_Visibility,
        Sphere, 
        QueryParams
    );
	
	AActor* NewTarget = nullptr;
	
	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		
		float DistanceToCharacter = FVector::Dist(OwnerCharacter->GetActorLocation(), HitResult.ImpactPoint);
		float MaxReachDistance = 250.0f;
		
		if (DistanceToCharacter <= MaxReachDistance)
		{
			if (HitActor->Implements<UNCInteractableInterface>())
			{
				if (INCInteractableInterface::Execute_CanInteract(HitActor, OwnerCharacter))
				{
					NewTarget = HitActor;
				}
			}
		}
	}
	
	if (NewTarget != CurrentInteractableTarget)
	{
		if (CurrentInteractableTarget)
		{
			SetHighlight(CurrentInteractableTarget, false);
		}
		CurrentInteractableTarget = NewTarget;
		
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
