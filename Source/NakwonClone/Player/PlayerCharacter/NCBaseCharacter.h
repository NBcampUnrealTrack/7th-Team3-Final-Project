#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "NCBaseCharacter.generated.h"

class UNCStatComponent;
class UAbilitySystemComponent;
class UVGPlayerAttributeSet;

UCLASS()
class NAKWONCLONE_API ANCBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANCBaseCharacter();

	// GAS ?명꽣?섏씠??
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UVGPlayerAttributeSet* GetPlayerAttributeSet() const;

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

	// TODO: GAS ?꾩쟾 ?꾪솚 ???쒓굅
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr<UNCStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector LandVelocity = FVector::ZeroVector;

	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	virtual void OnDead();

protected:
	//GAS
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UVGPlayerAttributeSet> PlayerAttributeSet;

protected:
	FTimerHandle LandingTimerHandle;
	void ResetLandingState();
};