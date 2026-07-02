#include "NCInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Item/NCItemActor.h"
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
		//헌호수정 - 이미 줍는 중이면 중복 실행 방지
		if (bIsLooting)
		{
			return;
		}

		ANCItemActor* Item = Cast<ANCItemActor>(CurrentInteractableTarget);

		//헌호수정 - 몽타주가 있으면: 몽타주 재생 → 손에 부착 → 몽타주 끝날 때 실제 획득
		if (LootMontage && OwnerCharacter)
		{
			bIsLooting = true;
			PendingLootTarget = Item;

			OwnerCharacter->PlayAnimMontage(LootMontage);

			// 손에 임시 메시 부착 + 바닥 아이템 숨김
			AttachLootMeshToHand(Item);

			// 몽타주 종료 콜백 바인딩 (성공/중단 분기)
			if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr)
			{
				AnimInst->OnMontageEnded.RemoveDynamic(this, &UNCInteractionComponent::OnLootMontageEndedInternal);
				AnimInst->OnMontageEnded.AddDynamic(this, &UNCInteractionComponent::OnLootMontageEndedInternal);
			}
			return;
		}

		//헌호수정 - 몽타주 없으면 기존처럼 즉시 획득 (폴백)
		INCInteractableInterface::Execute_Interact(Item, GetOwner());
		UpdateInteractableTarget();
		return;
	}

	INCInteractableInterface::Execute_Interact(CurrentInteractableTarget, GetOwner());
}

//헌호수정 - 줍기 몽타주 종료 시: 중단이면 취소, 정상 종료면 실제 획득
void UNCInteractionComponent::OnLootMontageEndedInternal(UAnimMontage* Montage, bool bInterrupted)
{
	// 줍기 몽타주가 아니면 무시
	if (Montage != LootMontage)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (UAnimInstance* AnimInst = OwnerCharacter && OwnerCharacter->GetMesh() ? OwnerCharacter->GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInst->OnMontageEnded.RemoveDynamic(this, &UNCInteractionComponent::OnLootMontageEndedInternal);
	}

	// 손에 붙인 임시 메시 정리
	ClearHeldItemMesh();

	ANCItemActor* Item = PendingLootTarget.Get();
	PendingLootTarget = nullptr;
	bIsLooting = false;

	// 중단(스턴/이동/죽음 등)되었거나 아이템이 이미 사라졌으면 획득 취소 (바닥 아이템 복구)
	if (bInterrupted || !IsValid(Item))
	{
		if (IsValid(Item) && Item->ItemMesh)
		{
			Item->ItemMesh->SetVisibility(true); // 숨겨둔 바닥 아이템 다시 표시
		}
		return;
	}

	// 정상 종료 → 실제 획득 처리 (서버 로직 그대로 호출)
	INCInteractableInterface::Execute_Interact(Item, GetOwner());
	UpdateInteractableTarget();
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

	// 임시 메시 컴포넌트 생성 (총기 장착 방식과 동일)
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
		LootHandSocketName);

	// 바닥 원본 아이템은 잠시 숨김 (성공 시 곧 파괴됨, 중단 시 다시 표시)
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

void UNCInteractionComponent::StopInteraction() //헌호수정 - 사망 시 호출
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_UpdateInteractable);
	if (CurrentInteractableTarget)
		SetHighlight(CurrentInteractableTarget, false);
	CurrentInteractableTarget = nullptr;
	bIsLooting = false;

	//헌호수정 - 줍는 도중 사망 시 임시 메시 정리 + 바닥 아이템 복구
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
