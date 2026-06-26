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

	// 헌호수정 - 무기별 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assassination")
	float AssassinationRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assassination")
	TObjectPtr<UAnimMontage> AssassinationMontage;

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
