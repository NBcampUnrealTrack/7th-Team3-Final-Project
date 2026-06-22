#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
#include "NCDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UCurveFloat;
class USoundBase;

UCLASS()
class NAKWONCLONE_API ANCDoor : public AActor, public INCInteractableInterface
{
	GENERATED_BODY()

public:
	ANCDoor();

protected:
	virtual void BeginPlay() override;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UTimelineComponent* DoorTimeline;

	// 디자이너 설정
	// 시간 = 열리는 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Setup")
	UCurveFloat* DoorCurve;

	// 열리는 각도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Setup")
	float OpenYaw = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Sound")
	USoundBase* OpenSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Sound")
	USoundBase* CloseSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Highlight")
	int32 HighlightStencilValue = 1;

	// 상태
	UPROPERTY(BlueprintReadOnly, Category = "Door|State")
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "Door|State")
	bool bIsAnimating = false;

	UPROPERTY(BlueprintReadOnly, Category = "Door|State")
	bool bIsPlayerInRange = false;

	// 타임라인 콜백
	UFUNCTION()
	void HandleDoorProgress(float Value);

	UFUNCTION()
	void HandleDoorFinished();

	// 오버랩
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void ToggleHighlight_Implementation(bool bHighlight) override;
	virtual FText GetInteractPrompt_Implementation() override;
};