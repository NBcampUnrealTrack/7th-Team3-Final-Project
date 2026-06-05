#include "UNCCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "NakwonClone/Framwork/GameInstacne/NCGameInstance.h"
#include "NakwonClone/Common/NCGameplayTags.h"

UNCCombatComponent::UNCCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true); // 컴포넌트 자체 Replicated 설정
}

void UNCCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<ANCBaseCharacter>(GetOwner());
    if (OwnerCharacter)
    {
        ASC = OwnerCharacter->GetAbilitySystemComponent();
    }
}

void UNCCombatComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UNCCombatComponent, EquippedWeapon);
}

//클라이언트에서 호출 → 서버로 전달
void UNCCombatComponent::EquipWeapon(FNCWeaponInstance WeaponInstance)
{
    if (!WeaponInstance.IsValid()) return;
    Server_EquipWeapon(WeaponInstance);
}

void UNCCombatComponent::UnEquipWeapon()
{
    if (!bIsEquipped) return;
    Server_UnEquipWeapon();
}

void UNCCombatComponent::ReduceDurability(float Amount)
{
    if (!bIsEquipped) return;
    Server_ReduceDurability(Amount);
}

//서버 RPC 구현
void UNCCombatComponent::Server_EquipWeapon_Implementation(
    FNCWeaponInstance WeaponInstance)
{
    Internal_EquipWeapon(WeaponInstance);
}

void UNCCombatComponent::Server_UnEquipWeapon_Implementation()
{
    Internal_UnEquipWeapon();
}

void UNCCombatComponent::Server_ReduceDurability_Implementation(float Amount)
{
    EquippedWeapon.CurrentDurability -= Amount;
    EquippedWeapon.CurrentDurability =
        FMath::Max(0.f, EquippedWeapon.CurrentDurability);

    //내구도 0이면 파손 처리
    if (EquippedWeapon.CurrentDurability <= 0.f)
    {
        EquippedWeapon.bIsBroken = true;
        if (ASC)
            ASC->AddLooseGameplayTag(NCWeapon::State_Broken);
    }
    //DOREPLIFETIME으로 자동 동기화됨
}

//실제 장착 처리 (서버에서만 실행)
void UNCCombatComponent::Internal_EquipWeapon(FNCWeaponInstance WeaponInstance)
{
    EquippedWeapon = WeaponInstance;
    bIsEquipped = true;

    if (ASC)
    {
        FNCWeaponData* Data = GetEquippedWeaponData();
        if (Data)
        {
            ASC->AddLooseGameplayTag(Data->WeaponTypeTag);
            ASC->AddLooseGameplayTag(Data->WeightTag);
            ASC->AddLooseGameplayTag(NCWeapon::State_Equipped);
        }
    }
}

void UNCCombatComponent::Internal_UnEquipWeapon()
{
    if (ASC)
    {
        FNCWeaponData* Data = GetEquippedWeaponData();
        if (Data)
        {
            ASC->RemoveLooseGameplayTag(Data->WeaponTypeTag);
            ASC->RemoveLooseGameplayTag(Data->WeightTag);
            ASC->RemoveLooseGameplayTag(NCWeapon::State_Equipped);
        }
    }

    EquippedWeapon = FNCWeaponInstance();
    bIsEquipped = false;
}

//OnRep - 클라이언트에서 장착 상태 동기화
void UNCCombatComponent::OnRep_EquippedWeapon()
{
    //파손 태그 동기화
    if (EquippedWeapon.bIsBroken && ASC)
        ASC->AddLooseGameplayTag(NCWeapon::State_Broken);
}

FNCWeaponData* UNCCombatComponent::GetEquippedWeaponData() const
{
    if (!bIsEquipped || !OwnerCharacter) return nullptr;

    UNCGameInstance* GI = Cast<UNCGameInstance>(
        OwnerCharacter->GetGameInstance());
    if (!GI) return nullptr;

    return GI->GetWeaponData(EquippedWeapon.WeaponID);
}

bool UNCCombatComponent::CanAttack() const
{
    if (!bIsEquipped) return false;
    if (EquippedWeapon.bIsBroken) return false;
    if (!ASC) return false;

    if (ASC->HasMatchingGameplayTag(NCWeapon::Action_Attacking)) return false;
    if (ASC->HasMatchingGameplayTag(NCWeapon::Action_Swapping)) return false;

    return true;
}