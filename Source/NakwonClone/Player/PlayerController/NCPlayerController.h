#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NCPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AANCLootBoxActor;
class UNCLootBoxHud;
class UNCEquipmentComponent;
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
	void QuickSlot4();
	void UnArm();
	// ----------
	
	void Attack();

	// 헌호수정 - 플래시라이트 토글
	void ToggleFlashlight();

	// 하상빈 추가 - 총기 입력
	UNCEquipmentComponent* GetGunComp() const;

	UFUNCTION()
	void OnGunSwapCompleted(ENCGunSlot NewSlot);

	// H키 맨손 전환 시 근접 자동장착 콜백 스킵용
	bool bUnArmPending = false;
	void GunStartFire();
	void GunStopFire();
	void GunStartADS();
	void GunStopADS();
	void GunReload();
	void GunToggleFireMode();
	void GunSelectPrimary();
	void GunSelectSecondary();
	void GunSelectMelee();
	// 헌호수정 - 암살
	void Assassinate();

	// 헌호수정 - 공격 중 여부 체크
	bool IsAttacking() const;

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
	TObjectPtr<UInputAction> QuickSlot4Action;
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

	// 하상빈 추가 - 총기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunFireAction;       // LMB

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunADSAction;        // RMB

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunReloadAction;     // R

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunToggleFireModeAction; // B

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunSlot1Action;      // 1 — 주무기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunSlot2Action;      // 2 — 보조무기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Gun")
	TObjectPtr<UInputAction> GunSlot3Action;      // 3 — 근접무기
	// 헌호수정 - 암살 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AssassinateAction;

};