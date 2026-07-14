
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NakwonClone/Common/NCInteractableInterface.h"
#include "NCItemActor.generated.h"

class UWidgetComponent;
class UStaticMeshComponent;
class UStaticMesh;
class USphereComponent;
class URotatingMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;

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
	/*UPROPERTY(VisibleAnywhere)
	UWidgetComponent* InteractionWidget;*/
	
#pragma region 시환 수정	
public:
	// 오버랩 시 자동 습득을 위한 트리거
	UPROPERTY(VisibleAnywhere, Category = "Item|Pickup")
	USphereComponent* PickupSphere;
	
	UPROPERTY(EditAnywhere, Category = "Item|Pickup")
	float PickupRadius = 100.f;
	
	// 배치/드롭 시 공중에서 회전
	UPROPERTY(VisibleAnywhere, Category = "Item|Pickup")
	URotatingMovementComponent* PickupRotation;

	// 상시 아우라 파티클
	UPROPERTY(VisibleAnywhere, Category = "Item|Pickup")
	UNiagaraComponent* IdleAuraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pickup")
	UNiagaraSystem* IdleAuraEffect;

	// 습득 순간 1회성 이펙트/사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pickup")
	UNiagaraSystem* PickupEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Pickup")
	USoundBase* PickupSound;
	
	// 습득 이펙트/사운드를 아이템 자리(false)가 아닌 습득한 액터(플레이어) 위치(true)에서 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Pickup")
	bool bPickupEffectAtInteractor = false;

	// 습득 성공 시 이펙트/사운드 재생 후 파괴
	UFUNCTION(BlueprintCallable, Category = "Item|Pickup")
	void ConsumeItem(AActor* Interactor = nullptr);

	// true면 습득 시 파괴하지 않고 RespawnTime 후 같은 자리에 다시 등장 (레벨 배치 아이템용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Respawn")
	bool bRespawnEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Respawn", meta = (EditCondition = "bRespawnEnabled", ClampMin = "0.1", Units = "s"))
	float RespawnTime = 30.f;
	
protected:
	UFUNCTION()
	void HandlePickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayPickupFX(AActor* Interactor);
	
	bool bAutoPickupEnabled = true;
	
	// 스폰 직후(드롭 위치가 플레이어와 겹쳐있는 순간) 즉시 재습득되는 걸 막기 위한 유예시간
	UPROPERTY(EditAnywhere, Category = "Item|Pickup")
	float PickupGraceDelay = 1.f;

	FTimerHandle PickupGraceTimerHandle;

	void EnablePickupSphere();

	FTimerHandle RespawnTimerHandle;

	void RespawnItem();
	
protected:
	virtual void Tick(float DeltaTime) override;   // 추가

	// 습득 시 아우라 확대/축소 펄스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Pickup", meta = (ClampMin = "1.0"))
	float AuraPulseScaleMultiplier = 4.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Pickup", meta = (ClampMin = "0.01"))
	float AuraPulseGrowTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Pickup", meta = (ClampMin = "0.01"))
	float AuraPulseShrinkTime = 0.15f;

	void PlayAuraPulse();
	void FinishConsume();

private:
	FVector AuraBaseScale = FVector::OneVector;
	float AuraPulseElapsed = 0.f;
	bool bAuraPulsing = false;

	FTimerHandle ConsumeTimerHandle;
#pragma endregion
	
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Data")
	FDataTableRowHandle ItemRowHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	class UStaticMeshComponent* ItemMesh;
	
	// 아이템 수량 (기본값 : 1)
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Data")
	int32 Quantity = 1;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item|Data")
	FGameplayTag ItemTypeTag;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Data")
	FName ItemID; 

	UPROPERTY(ReplicatedUsing = OnRep_ItemMeshAsset, BlueprintReadOnly, Category = "Item|Data")
	class UStaticMesh* ItemMeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Mesh")
	FRotator DefaultMeshRotation = FRotator::ZeroRotator;

	UFUNCTION()
	void OnRep_ItemMeshAsset();
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeItemData(FName InItemID, FGameplayTag InTag, int32 InQuantity, UStaticMesh* InMesh);

	// 아이템 사용
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void UseItem(class ACharacter* User);
	
public:
	virtual void Interact_Implementation(AActor* Interactor) override;
    
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
    
	virtual FText GetInteractPrompt_Implementation() override;
	
	virtual void ToggleHighlight_Implementation(bool bHighlight) override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
};
