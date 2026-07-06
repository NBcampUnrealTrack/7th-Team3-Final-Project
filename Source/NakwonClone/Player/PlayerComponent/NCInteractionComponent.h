#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NCInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetChangedSignature, AActor*, NewTarget);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCInteractionComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void Interact();

	// 헌호수정 - 사망 시 상호작용 완전 비활성화
	UFUNCTION(BlueprintCallable, Category = "Interaction|Action")
	void StopInteraction();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Animation")
	void OnLootMontageEnded();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Animation")
	void AttachPendingLootToHand();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Animation")
	void StorePendingLoot();

	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractTargetChangedSignature OnInteractTargetChanged;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Animation")
	TObjectPtr<UAnimMontage> PickupMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Animation")
	TObjectPtr<UAnimMontage> PickupStoreMontage;

	//헌호수정 - 줍기 시 아이템을 붙일 손 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Animation")
	FName LootHandSocketName = TEXT("hand_ItemSocket");

private:
	FTimerHandle TimerHandle_UpdateInteractable;

	//헌호수정 - 줍기 몽타주 진행 중 상태
	// 몽타주 끝날 때 실제 획득할 대상 (도중에 파괴될 수 있어 WeakPtr)
	TWeakObjectPtr<class ANCItemActor> PendingLootTarget;

	bool bLootStored = false;

	// 손에 붙이는 임시 시각용 메시 (총기 장착 방식과 동일, 리플리케이션 충돌 방지)
	UPROPERTY()
	TObjectPtr<class UStaticMeshComponent> HeldItemMeshComp;

	//헌호수정 - 몽타주 종료 콜백 (성공/중단 분기)
	UFUNCTION()
	void OnLootMontageEndedInternal(class UAnimMontage* Montage, bool bInterrupted);

	//헌호수정 - 손에 임시 메시 부착 / 정리
	void AttachLootMeshToHand(class ANCItemActor* Item);
	void ClearHeldItemMesh();
	
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Settings")
	float InteractionCheckInterval = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Settings")
	float InteractionSearchRadius = 200.0f;
	
	UPROPERTY()
	AActor* CurrentInteractableTarget = nullptr;
	
	bool bIsLooting = false;
	
	void UpdateInteractableTarget();
	
	void SetHighlight(AActor* TargetActor, bool bHighlight);
	
};
