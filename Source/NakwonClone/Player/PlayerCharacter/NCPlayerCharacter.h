#pragma once

#include "CoreMinimal.h"
#include "NCBaseCharacter.h"

#include "NCPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaBarChanged, float, CurrentStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotImageChanged, UTexture2D*, ItemIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCitizenRankTextChanged, int32, CitizenRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTextChanged, int32, CharacterLevel);

UCLASS()
class NAKWONCLONE_API ANCPlayerCharacter : public ANCBaseCharacter
{
	GENERATED_BODY()

public:
	ANCPlayerCharacter();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	UPROPERTY()
	FOnHPChanged OnHPChanged;
	
	UPROPERTY()
	FOnStaminaBarChanged OnStaminaBarChanged;
	
	UPROPERTY()
	FOnSlotImageChanged OnLeftSlotImageChanged;
	
	UPROPERTY()
	FOnSlotImageChanged OnRightSlotImageChanged;
	
	UPROPERTY()
	FOnCitizenRankTextChanged OnCitizenRankTextChanged;
	
	UPROPERTY()
	FOnLevelTextChanged OnLevelTextChanged;
	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	UFUNCTION(Server, Reliable)
	void Server_SetGait(FGameplayTag NewGaitTag);
	
	UFUNCTION(Server, Reliable)
	void Server_SetStance(FGameplayTag NewStanceTag);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* WalkAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* CrouchAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void StartSprint();
	void StopSprint();
	
	void ToggleWalk();
	void ToggleCrouch();
};
