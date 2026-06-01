#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCCreditItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCCreditItemActor : public ANCItemActor
{
    GENERATED_BODY()

public:
    ANCCreditItemActor();

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Credit")
    int32 CreditValue;

    virtual void UseItem(class ACharacter* User) override;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};