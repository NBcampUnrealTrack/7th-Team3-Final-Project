#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "UNCCombatComponent.generated.h"

class ANCBaseCharacter;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NAKWONCLONE_API UNCCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNCCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//무기 장착 (클라이언트에서 호출 → 서버로 전달)
	void EquipWeapon(FNCWeaponInstance WeaponInstance);

	//무기 해제
	void UnEquipWeapon();

	//현재 장착 무기 반환
	FNCWeaponInstance GetEquippedWeapon() const { return EquippedWeapon; }

	//장착 무기 DataTable 데이터 반환
	FNCWeaponData* GetEquippedWeaponData() const;

	//공격 가능 여부
	bool CanAttack() const;

	//내구도 감소 (서버에서만 호출)
	void ReduceDurability(float Amount);

private:
	//서버 RPC
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(FNCWeaponInstance WeaponInstance);

	UFUNCTION(Server, Reliable)
	void Server_UnEquipWeapon();

	UFUNCTION(Server, Reliable)
	void Server_ReduceDurability(float Amount);

	//실제 장착 처리
	void Internal_EquipWeapon(FNCWeaponInstance WeaponInstance);
	void Internal_UnEquipWeapon();

	//OnRep - 장착 무기 동기화
	UFUNCTION()
	void OnRep_EquippedWeapon();

private:
	//캐싱
	UPROPERTY()
	TObjectPtr<ANCBaseCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	//현재 장착된 무기 인스턴스 (Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	FNCWeaponInstance EquippedWeapon;

	//장착 여부
	bool bIsEquipped = false;
};