#include "GE_Damage.h"
#include "NakwonClone/GAS/AttributeSet/VGMonsterAttributeSet.h"
#include "NakwonClone/Common/NCGameplayTags.h"

UGE_Damage::UGE_Damage()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // SetByCaller로 받은 데미지 값을 Health에 감소 적용
    FGameplayModifierInfo ModifierInfo;
    ModifierInfo.Attribute = UVGMonsterAttributeSet::GetHealthAttribute();
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;

    FSetByCallerFloat SetByCaller;
    SetByCaller.DataTag = NCData::Damage;
    ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

    Modifiers.Add(ModifierInfo);
}
