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

	/** 사용 시 회복할 체력량 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Heal")
	float HealAmount;

	virtual void UseItem(class ACharacter* User) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};