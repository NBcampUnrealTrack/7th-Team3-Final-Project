#include "NCTutorialTrigger.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Character.h"

ANCTutorialTrigger::ANCTutorialTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->InitBoxExtent(FVector(100.f, 100.f, 100.f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    RootComponent = TriggerBox;
}

void ANCTutorialTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ANCTutorialTrigger::OnOverlapBegin);
}

void ANCTutorialTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bHasTriggered && bTriggerOnce) return;
    if (!Cast<ACharacter>(OtherActor)) return;
    if (StepIDs.Num() == 0 || !KeyGuideWidgetClass) return;

    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, KeyGuideWidgetClass, false);

    UUserWidget* KeyGuideWidget = nullptr;
    if (FoundWidgets.Num() > 0)
    {
        KeyGuideWidget = FoundWidgets[0];
    }
    else
    {
        APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        if (!PC) return;

        KeyGuideWidget = CreateWidget<UUserWidget>(PC, KeyGuideWidgetClass);
        if (KeyGuideWidget)
        {
            KeyGuideWidget->AddToViewport();
        }
    }

    if (!KeyGuideWidget) return;

    if (UFunction* ShowStepsFunc = KeyGuideWidget->FindFunction(TEXT("ShowSteps")))
    {
        struct FShowStepsParams { TArray<FName> StepIDs; } Params{ StepIDs };
        KeyGuideWidget->ProcessEvent(ShowStepsFunc, &Params);
        bHasTriggered = true;
    }
}
