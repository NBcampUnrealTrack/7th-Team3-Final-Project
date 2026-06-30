#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NCAssassinationComponent.generated.h"

class AVGMonsterCharacterBase;
class ACameraActor;
class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCAssassinationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCAssassinationComponent();

	// 헌호수정 - 암살 시도 (외부 진입점)
	void TryAssassinate();

	// 헌호수정 - 암살 범위 (무기 데이터와 무관하게 고정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assassination")
	float AssassinationRange = 200.f;

private:
	AVGMonsterCharacterBase* FindNearestTarget() const;
	void StartSlowMo();
	void StartCamera(AVGMonsterCharacterBase* Target);
	void Finish();

	UPROPERTY()
	TObjectPtr<ACameraActor> AssassinationCamera;

	FTimerHandle KillTimerHandle;
	FTimerHandle CameraTimerHandle;
};
