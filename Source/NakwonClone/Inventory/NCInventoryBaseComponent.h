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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	TArray<FInventorySlot> Items;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void InitializeInventory();
};