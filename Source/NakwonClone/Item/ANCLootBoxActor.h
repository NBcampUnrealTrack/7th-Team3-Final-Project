#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NakwonClone/Common/NCInteractableInterface.h"

#include "ANCLootBoxActor.generated.h"

class UStaticMeshComponent;
class UNCInventoryBaseComponent;
class UDataTable;

UCLASS()
class NAKWONCLONE_API AANCLootBoxActor : public AActor, public INCInteractableInterface
{
	GENERATED_BODY()

public:
	AANCLootBoxActor();
	
protected:
	virtual void BeginPlay() override;
	
	void GenerateLoot();
	
public:
	UFUNCTION(BlueprintCallable, Category = "LootBox|Interaction")
	void Interact(AActor* Interactor);
	
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual void ToggleHighlight_Implementation(bool bHighlight) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LootBox|Components")
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LootBox|Components")
	TObjectPtr<UNCInventoryBaseComponent> LootInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LootBox|Settings")
	TMap<FName, int32> FixedLootItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LootBox|Settings")
	TObjectPtr<UDataTable> RandomDropTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LootBox|Settings", meta = (ClampMin = "0"))
	int32 RandomRollCount = 1;
	
};