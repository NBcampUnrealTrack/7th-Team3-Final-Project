
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NakwonClone/Common/NCInteractableInterface.h"

#include "NCItemActor.generated.h"

UCLASS()
class NAKWONCLONE_API ANCItemActor : public AActor, public INCInteractableInterface
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
	int32 Quantity = 1;
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeItemData(FGameplayTag InTag, int32 InQuantity);

	// 아이템 사용
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void UseItem(class ACharacter* User);
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;
    
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
    
	virtual FText GetInteractPrompt_Implementation() override;
	
	virtual void ToggleHighlight_Implementation(bool bHighlight) override;
};
