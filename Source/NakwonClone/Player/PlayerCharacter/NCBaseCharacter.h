#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "NCBaseCharacter.generated.h"

class UNCStatComponent;


UCLASS()
class NAKWONCLONE_API ANCBaseCharacter : public ACharacter, public IAbilitySystemInterface 
{
	GENERATED_BODY()

public:
	ANCBaseCharacter();
	
#pragma region 추가된 부분 GAS
public:
	// IAbilitySystemInterface 구현
	// IAbilitySystemInterface : GAS가 제공하는 인터페이스
	// 이걸 상속 받아야 GAS 시스템이 해당 액터에 ASC를 갖고 있는지 판별 가능
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
#pragma endregion

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	FGameplayTag CurrentGaitTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	FGameplayTag CurrentStanceTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "State")
	FGameplayTag CurrentActionTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr<UNCStatComponent> StatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector LandVelocity = FVector::ZeroVector;

	virtual void Landed(const FHitResult& Hit) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	class UDataTable* MovementDataTable;

	void ApplyMovementData(FGameplayTag StateTag);

protected:
	FTimerHandle LandingTimerHandle;
	void ResetLandingState();
};