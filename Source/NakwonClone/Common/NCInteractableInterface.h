#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "NCInteractableInterface.generated.h"

UINTERFACE()
class UNCInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NAKWONCLONE_API INCInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractPrompt();
};
