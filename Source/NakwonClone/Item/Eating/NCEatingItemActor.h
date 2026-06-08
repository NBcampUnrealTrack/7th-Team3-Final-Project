#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCEatingItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCEatingItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCEatingItemActor();

	// 체력 회복량/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Eating")
	float HealAmount;

	// 스태미나 회복량
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Eating")
	float StaminaAmount;

	// 감염도 감소량
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Eating")
	float InfectionReduceAmount;

	virtual void UseItem(class ACharacter* User) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};