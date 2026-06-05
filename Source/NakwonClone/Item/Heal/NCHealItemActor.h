#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCHealItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCHealItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCHealItemActor();

	//체력 회복량
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Heal")
	float HealAmount;

	// 스태미나 회복량
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Heal")
	float StaminaAmount;

	// 감염도 감소량
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Heal")
	float InfectionReduceAmount;

	virtual void UseItem(class ACharacter* User) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};