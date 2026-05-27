#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NCInventoryType.h"

#include "NCInventoryBaseComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCInventoryBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCInventoryBaseComponent();

protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	FIntPoint GridSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = Inventory)
	TArray<FInventorySlot> Items;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void InitializeInventory();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory)
	TObjectPtr<class UDataTable> ItemDataTable;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual bool AddItem(FGameplayTag ItemTypeTag, int32 Quantity);
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool FindEmptySlot(int32& OutSlotIndex) const;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool FindStackableSlot(FGameplayTag ItemTypeTag, int32 MaxStackSize, int32& OutSlotIndex) const;
};