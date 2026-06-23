#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NCPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AANCLootBoxActor;
class UNCLootBoxHud;
struct FInputActionValue;

//하상빈 추가
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryToggledSignature, bool, bIsOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPauseRequestedSignature);

UCLASS()
class NAKWONCLONE_API ANCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANCPlayerController();

	//하상빈 추가
	UPROPERTY(BlueprintAssignable, Category = "UI|Inventory")
	FOnInventoryToggledSignature OnInventoryToggled;
	UFUNCTION(Client, Reliable)
	void Client_OpenLootBoxUI(AANCLootBoxActor* TargetBox);
	void ToggleInventory();
	void CloseLootBoxUI();
		
	UFUNCTION(BlueprintCallable)
	bool TryCloseTopUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HandleCloseUI();

	UPROPERTY(BlueprintAssignable, Category = "UI|Pause")
	FOnPauseRequestedSignature OnPauseRequested;

	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void SetPauseMenuOpen(bool bOpen);

	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsMenuBlockingInput() const { return bIsInventoryOpen || LootBoxWidget != nullptr || bIsPauseMenuOpen; }

	UPROPERTY(BlueprintReadOnly, Category = "UI|Pause")
	bool bIsPauseMenuOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInventoryOpen = false;
	
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
	void OnInventoryKey();
	void Interact();
	void QuickSlot1();
	void QuickSlot2();
	void QuickSlot3();
	void QuickSlot3Hold();
	void UnArm();
	// ----------
	
	void Attack();

	// 헌호수정 - 플래시라이트 토글
	void ToggleFlashlight();

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
	
	// ---하상빈 추가---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickSlot1Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickSlot2Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickSlot3Action;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickSlot3HoldAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> UnArmAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CloseUIAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|LootBox")
	TSubclassOf<UNCLootBoxHud> LootBoxWidgetClass;
	UPROPERTY()
	TObjectPtr<UNCLootBoxHud> LootBoxWidget;
	// ----------
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	// 헌호수정 - 플래시라이트 토글 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> FlashlightAction;

};