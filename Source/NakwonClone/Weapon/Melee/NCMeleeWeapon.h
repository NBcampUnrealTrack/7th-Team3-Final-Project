#pragma once

#include "CoreMinimal.h"
#include "NakwonClone/Weapon/NCWeaponBase.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "GameplayEffect.h"
#include "NCMeleeWeapon.generated.h"

class ANCBaseCharacter;

UCLASS()
class NAKWONCLONE_API ANCMeleeWeapon : public ANCWeaponBase
{
	GENERATED_BODY()

public:
	ANCMeleeWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	void InitFromDataTable(FName RowName);

	UFUNCTION(BlueprintCallable)
	void StartTrace();

	UFUNCTION(BlueprintCallable)
	void EndTrace();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UDataTable> WeaponDataTable;

	FNCWeaponData CurrentWeaponData;

	//GAS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

private:
	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(ANCBaseCharacter* Target);

	void PerformTrace();

	//디버그를 모든 클라이언트에서 보여주기 위한 Multicast
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DrawDebug(FVector Start, FVector End);

	bool bIsTracing = false;

	FVector PreviousStart;
	FVector PreviousEnd;

	TArray<TObjectPtr<AActor>> HitActors;
};