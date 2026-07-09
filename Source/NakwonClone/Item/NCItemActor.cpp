#include "NCItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "NakwonClone/Player/PlayerComponent/NCPlayerInventoryComponent.h"

ANCItemActor::ANCItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(SceneRoot);

	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ItemMesh->SetCollisionObjectType(ECC_WorldStatic);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	/*InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(ItemMesh);
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	InteractionWidget->SetVisibility(false);*/
	
	// ─── 시환 추가
	// 오버랩 자동 습득용 트리거
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(SceneRoot);
	PickupSphere->SetSphereRadius(PickupRadius);
	// 등록 시점 즉시습득 방지: 일단 꺼진 채로 시작
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupSphere->SetCollisionObjectType(ECC_WorldStatic);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ANCItemActor::HandlePickupOverlap);
	
	// 배치/드롭 시 공중 회전 연출
	PickupRotation = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("PickupRotation"));
	PickupRotation->RotationRate = FRotator(0.f, 90.f, 0.f);

	// 상시 아우라 파티클 (에셋 미지정 시 자동 활성화 안 함)
	IdleAuraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IdleAuraComponent"));
	IdleAuraComponent->SetupAttachment(ItemMesh);
	IdleAuraComponent->bAutoActivate = false;
}

void ANCItemActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (IdleAuraEffect && IdleAuraComponent)
	{
		IdleAuraComponent->SetAsset(IdleAuraEffect);
		IdleAuraComponent->Activate();
	}
	
	if (bAutoPickupEnabled && PickupSphere)
	{
		GetWorld()->GetTimerManager().SetTimer(
			PickupGraceTimerHandle,
			this,
			&ANCItemActor::EnablePickupSphere,
			PickupGraceDelay,
			false);
	}
}

void ANCItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCItemActor, ItemTypeTag);
	DOREPLIFETIME(ANCItemActor, Quantity);
	DOREPLIFETIME(ANCItemActor, ItemID);    
	DOREPLIFETIME(ANCItemActor, ItemMeshAsset);
}

// 자식 클래스에서 오버라이드
void ANCItemActor::UseItem(ACharacter* User)
{
	
}

void ANCItemActor::HandlePickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	ANCPlayerCharacter* Player = Cast<ANCPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	Execute_Interact(this, Player);
}

void ANCItemActor::Multicast_PlayPickupFX_Implementation()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation());
	}

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
	}
}

void ANCItemActor::ConsumeItem()
{
	Multicast_PlayPickupFX();
	Destroy();
}

void ANCItemActor::Interact_Implementation(AActor* Interactor)
{
	ANCPlayerCharacter* PlayerCharacter = Cast<ANCPlayerCharacter>(Interactor);
	if (!PlayerCharacter)
	{
		return;
	}

	UNCPlayerInventoryComponent* InventoryComp = PlayerCharacter->GetInventoryComponent();
	if (InventoryComp)
	{
		InventoryComp->LootItem(this);
	}
}

bool ANCItemActor::CanInteract_Implementation(AActor* Interactor)
{
	// 임시로 무조건 상호작용 가능하도록 true 반환
	//return true;
	
	return false;
}

FText ANCItemActor::GetInteractPrompt_Implementation()
{
	// return FText::FromString(TEXT("아이템 줍기"));
	return FText::GetEmpty();
}

void ANCItemActor::ToggleHighlight_Implementation(bool bHighlight)
{
	/*ItemMesh->SetRenderCustomDepth(bHighlight);

	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(bHighlight);
	}
	
	if (ItemMesh) 
	{
		ItemMesh->SetRenderCustomDepth(bHighlight);
		ItemMesh->SetCustomDepthStencilValue(1);
	}*/
}

void ANCItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (PickupSphere)
	{
		PickupSphere->SetSphereRadius(PickupRadius);
	}

	if (!ItemID.IsNone())
	{
		UDataTable* LoadedItemDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/NakwonClone/Blueprints/Item/ItemData/DT_ItemTypeData.DT_ItemTypeData"));
		
		if (LoadedItemDataTable)
		{
			FItemData* FoundData = LoadedItemDataTable->FindRow<FItemData>(ItemID, TEXT("ActorConstruction"));
			
			if (FoundData)
			{
				ItemTypeTag    = FoundData->ItemTypeTag;
				PickupEffect   = FoundData->PickupEffect;
				PickupSound    = FoundData->PickupSound;
				IdleAuraEffect = FoundData->IdleAuraEffect;
				
				if (ItemMesh && FoundData->ItemMesh)
				{
					ItemMesh->SetStaticMesh(FoundData->ItemMesh);
				}
			}
		}
	}
}

void ANCItemActor::OnRep_ItemMeshAsset()
{
	if (ItemMesh && ItemMeshAsset)
	{
		ItemMesh->SetStaticMesh(ItemMeshAsset);
	}
}

void ANCItemActor::InitializeItemData(FName InItemID, FGameplayTag InTag, int32 InQuantity, UStaticMesh* InMesh)
{
	if (HasAuthority())
	{
		ItemID = InItemID;
		ItemTypeTag = InTag;
		Quantity = InQuantity;
		
		ItemMeshAsset = InMesh;
		OnRep_ItemMeshAsset();
	}
}

void ANCItemActor::EnablePickupSphere()
{
	if (PickupSphere)
	{
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}