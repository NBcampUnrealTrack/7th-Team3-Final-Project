#include "HitCheckNotify.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"
#include "NakwonClone/GAS/Effect/GE_Damage.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

void UHitCheckNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    ACharacter* OwnerChar = Cast<ACharacter>(MeshComp->GetOwner());
    if (!OwnerChar) return;

    // 서버에서만 판정 (멀티플레이어)
    if (!OwnerChar->HasAuthority()) return;

    UWorld* World = OwnerChar->GetWorld();
    if (!World) return;

    // CombatComponent에서 무기 데이터 가져오기
    UNCCombatComponent* Combat = OwnerChar->FindComponentByClass<UNCCombatComponent>();
    if (!Combat) return;

    FNCWeaponData* WeaponData = Combat->GetEquippedWeaponData();
    if (!WeaponData) return;

    // 구 트레이스 (HitBoxExtent.X를 반지름으로 사용)
    const FVector Start = OwnerChar->GetActorLocation();
    const FVector End = Start + OwnerChar->GetActorForwardVector() * 120.f;
    const float Radius = WeaponData->HitBoxExtent.X;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerChar);

    TArray<FHitResult> HitResults;
    bool bHit = World->SweepMultiByChannel(
        HitResults, Start, End, FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(Radius),
        Params
    );

    // 디버그 구 (3초)
    DrawDebugSphere(World, End, Radius, 12,
        bHit ? FColor::Red : FColor::Green, false, 3.f, 0, 2.f);

    // 히트된 좀비에 GAS로 데미지 적용
    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerChar);
    if (!SourceASC) return;

    for (const FHitResult& Hit : HitResults)
    {
        AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(Hit.GetActor());
        if (!Monster) continue;

        UAbilitySystemComponent* TargetASC = Monster->GetAbilitySystemComponent();
        if (!TargetASC) continue;

        // GE_Damage 생성 및 데미지 값 설정
        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(
            UGE_Damage::StaticClass(), 1.f, Context);

        if (Spec.IsValid())
        {
            // 데미지 값을 음수로 설정 (Health 감소)
            Spec.Data->SetSetByCallerMagnitude(NCData::Damage, -WeaponData->Damage);
            SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

            UE_LOG(LogTemp, Warning, TEXT("[HitCheckNotify] 좀비 데미지 적용: %.1f → %s"),
                WeaponData->Damage, *Monster->GetName());
        }
    }
}
