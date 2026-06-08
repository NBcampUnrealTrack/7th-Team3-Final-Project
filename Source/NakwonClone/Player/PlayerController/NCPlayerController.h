#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NCPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

//하상빈 추가
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryToggledSignature, bool, bIsOpen);

UCLASS()
class NAKWONCLONE_API ANCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANCPlayerController();

	//하상빈 추가
	UPROPERTY(BlueprintAssignable, Category = "UI|Inventory")
	FOnInventoryToggledSignature OnInventoryToggled;
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void ToggleWalk();
	void Jump();
	void StopJump();
	void ToggleCrouch();
	// 하상빈 추가
	void Interact();
	void ToggleInventory();

	void Attack();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	// 하상빈 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInventoryOpen = false;
};