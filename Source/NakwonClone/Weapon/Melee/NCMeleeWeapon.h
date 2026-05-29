#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Weapon/NCWeaponBase.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NCMeleeWeapon.generated.h"

UCLASS()
class NAKWONCLONE_API ANCMeleeWeapon : public ANCWeaponBase
{
	GENERATED_BODY()

public:
	ANCMeleeWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void InitFromDataTable(FName RowName);

	UFUNCTION(BlueprintCallable)
	void EnableHitBox();

	UFUNCTION(BlueprintCallable)
	void DisableHitBox();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UDataTable> WeaponDataTable;

	FNCMeleeWeaponData CurrentWeaponData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<class UBoxComponent> HitBox;

private:
	UFUNCTION()
	void OnHitBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// TODO: GAS 전환 시 → GameplayEffect 적용으로 대체
	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(ANCBaseCharacter* Target);

	UPROPERTY(Replicated)
	bool bHitBoxEnabled = false;
};