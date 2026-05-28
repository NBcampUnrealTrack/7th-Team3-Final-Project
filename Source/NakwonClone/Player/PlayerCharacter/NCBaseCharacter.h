#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "NCBaseCharacter.generated.h"

class UNCStatComponent;

UCLASS()
class NAKWONCLONE_API ANCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANCBaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	FGameplayTag CurrentGaitTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	FGameplayTag CurrentStanceTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	FGameplayTag CurrentActionTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr<UNCStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector LandVelocity = FVector::ZeroVector;

	virtual void Landed(const FHitResult& Hit) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	class UDataTable* MovementDataTable;

	void ApplyMovementData(FGameplayTag StateTag);

protected:
	FTimerHandle LandingTimerHandle;
	void ResetLandingState();
};