#include "NCDoor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"

ANCDoor::ANCDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	DoorMesh->SetupAttachment(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Box1"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);

	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));
}

void ANCDoor::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANCDoor::OnBoxBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ANCDoor::OnBoxEndOverlap);

	// 타임라인 바인딩
	if (DoorCurve)
	{
		FOnTimelineFloat ProgressDelegate;
		ProgressDelegate.BindUFunction(this, FName("HandleDoorProgress"));
		DoorTimeline->AddInterpFloat(DoorCurve, ProgressDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName("HandleDoorFinished"));
		DoorTimeline->SetTimelineFinishedFunc(FinishedDelegate);
	}
}

// Door 회전
void ANCDoor::HandleDoorProgress(float Value)
{
	DoorMesh->SetRelativeRotation(FRotator(0.f, Value * OpenYaw, 0.f));
}

// 애니메이션 잠금 해제
void ANCDoor::HandleDoorFinished()
{
	bIsOpen = !bIsOpen;
	bIsAnimating = false;
}

void ANCDoor::OnBoxBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*,
	int32, bool, const FHitResult&)
{
	bIsPlayerInRange = true;
}

void ANCDoor::OnBoxEndOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32)
{
	bIsPlayerInRange = false;
}

// 상호작용
void ANCDoor::Interact_Implementation(AActor* Interactor)
{
	// 애니메이션 중이면 무시
	if (bIsAnimating)
	{
		return;
	}
	bIsAnimating = true;

	if (bIsOpen)
	{
		// 닫기: 사운드 역재생
		if (CloseSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CloseSound, GetActorLocation());
		}
		DoorTimeline->Reverse();
	}
	else
	{
		// 열기: 사운드 → 정재생
		if (OpenSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
		}
		DoorTimeline->PlayFromStart();
	}
}

bool ANCDoor::CanInteract_Implementation(AActor* Interactor)
{
	return !bIsAnimating;
}

// 외곽선(CustomDepth) on/off
void ANCDoor::ToggleHighlight_Implementation(bool bHighlight)
{
	if (bHighlight)
	{
		DoorMesh->SetRenderCustomDepth(true);
		DoorMesh->SetCustomDepthStencilValue(HighlightStencilValue);
	}
	else
	{
		DoorMesh->SetRenderCustomDepth(false);
	}
}

FText ANCDoor::GetInteractPrompt_Implementation()
{
	return bIsOpen
		? FText::FromString(TEXT("닫기"))
		: FText::FromString(TEXT("열기"));
}