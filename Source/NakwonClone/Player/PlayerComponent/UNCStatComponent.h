#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NakwonClone/Player/PlayerData/NCPlayerStatData.h"
#include "UNCStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNCOnHPChanged, float, OldHP, float, NewHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNCOnDead);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCStatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void ApplyDamage(float DamageAmount);
	void ApplyHeal(float HealAmount);

	UFUNCTION(BlueprintCallable)
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable)
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintCallable)
	bool IsAlive() const { return bIsAlive; }

public:
	UPROPERTY(BlueprintAssignable)
	FNCOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable)
	FNCOnDead OnDead;

private:
	UFUNCTION()
	void OnRep_CurrentHP(float OldHP);

	void HandleDeath();

private:
	UPROPERTY(EditAnywhere, Category = "Stat")
	TObjectPtr<UDataTable> StatDataTable;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP;

	UPROPERTY(Replicated)
	float MaxHP;

	UPROPERTY(Replicated)
	float MaxStamina;

	UPROPERTY(Replicated)
	bool bIsAlive;
};