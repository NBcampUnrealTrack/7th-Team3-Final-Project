#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerComponent/NCGunComponent.h"
#include "NCShotgunComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCShotgunComponent : public UNCGunComponent
{
	GENERATED_BODY()

public:
	virtual void Reload() override;

	UFUNCTION(BlueprintCallable, Category = "Gun|Action")
	void StopReload();

protected:
	virtual void OnBeforeFire() override { StopReload(); }
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void OnShellInserted();

	FTimerHandle ShellTimerHandle;
};
