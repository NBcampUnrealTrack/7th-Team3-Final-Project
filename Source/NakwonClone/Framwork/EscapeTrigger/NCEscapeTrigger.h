#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NCEscapeTrigger.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class NAKWONCLONE_API ANCEscapeTrigger : public AActor
{
	GENERATED_BODY()

public:
	ANCEscapeTrigger();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Escape")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Escape")
	TSubclassOf<UUserWidget> ClearWidgetClass;

	// 탈출 시 이동할 레벨 (비우면 이동 안 함)
	UPROPERTY(EditAnywhere, Category = "Escape")
	TSoftObjectPtr<UWorld> NextLevel;

	// 이동 전 대기 시간 (클리어 위젯 잠깐 보여주려면 값 입력, 0 = 즉시)
	UPROPERTY(EditAnywhere, Category = "Escape")
	float LevelOpenDelay = 0.f;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void OpenNextLevel();

	FTimerHandle LevelOpenTimerHandle;
};