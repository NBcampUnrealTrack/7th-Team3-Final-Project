
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

#include "NCItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCItemActor : public AActor
{
	GENERATED_BODY()

public:
	ANCItemActor();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class UStaticMeshComponent* ItemMesh;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Data")
	FGameplayTag ItemTypeTag;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Data")
	int32 Quantity;
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeItemData(FGameplayTag InTag, int32 InQuantity);
};
