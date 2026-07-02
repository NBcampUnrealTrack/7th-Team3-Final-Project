
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NCParkinglotGameMode.generated.h"

UCLASS()
class NAKWONCLONE_API ANCParkinglotGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	ANCParkinglotGameMode();
	
	virtual void BeginPlay() override;
	
	FTimerHandle NextMapReadyTimerHandle;
	
public:
	UFUNCTION(BlueprintCallable)
	void CheckMapReady();
	
	UFUNCTION(BlueprintCallable)
	void MoveToShopLevel();
};
