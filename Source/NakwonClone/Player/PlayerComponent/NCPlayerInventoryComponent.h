#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"

#include "NCPlayerInventoryComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCPlayerInventoryComponent : public UNCInventoryBaseComponent
{
	GENERATED_BODY()

public:
	UNCPlayerInventoryComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    
	virtual void InitializeInventory() override;
public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Inventory|QuickSlot")
	TArray<FInventorySlot> QuickSlots;
};
