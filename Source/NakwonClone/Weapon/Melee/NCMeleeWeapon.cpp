#include "NCMeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "NakwonClone/Player/PlayerCharacter/NCBaseCharacter.h"
#include "NakwonClone/Player/PlayerComponent/UNCStatComponent.h"

ANCMeleeWeapon::ANCMeleeWeapon()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(WeaponMesh);
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &ANCMeleeWeapon::OnHitBoxOverlap);
}

void ANCMeleeWeapon::BeginPlay()
{
    Super::BeginPlay();
}

void ANCMeleeWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANCMeleeWeapon, bHitBoxEnabled);
}

void ANCMeleeWeapon::InitFromDataTable(FName RowName)
{
    if (!WeaponDataTable) return;

    FNCMeleeWeaponData* Data = WeaponDataTable->FindRow<FNCMeleeWeaponData>(
        RowName, TEXT("MeleeWeaponData"));

    if (Data)
    {
        CurrentWeaponData = *Data;
        HitBox->SetBoxExtent(CurrentWeaponData.HitBoxExtent);
    }
}

void ANCMeleeWeapon::EnableHitBox()
{
    if (!HasAuthority()) return;
    bHitBoxEnabled = true;
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ANCMeleeWeapon::DisableHitBox()
{
    if (!HasAuthority()) return;
    bHitBoxEnabled = false;
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANCMeleeWeapon::OnHitBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (OtherActor == GetOwner()) return;

    if (ANCBaseCharacter* Target = Cast<ANCBaseCharacter>(OtherActor))
    {
        Server_ApplyDamage(Target);
    }
}

// TODO: GAS 전환 시 → ApplyGameplayEffectToTarget으로 대체
void ANCMeleeWeapon::Server_ApplyDamage_Implementation(ANCBaseCharacter* Target)
{
    if (!Target) return;

    if (UNCStatComponent* StatComp = Target->StatComponent)
    {
        StatComp->ApplyDamage(CurrentWeaponData.Damage);
    }
}