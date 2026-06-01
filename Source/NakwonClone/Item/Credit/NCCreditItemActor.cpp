#include "NCCreditItemActor.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Common/NCGameplayTags.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"

ANCCreditItemActor::ANCCreditItemActor()
{
    CreditValue = 100;
    ItemTypeTag = NCItemTag::Credit;
}

void ANCCreditItemActor::UseItem(ACharacter* User)
{
    Super::UseItem(User);

    if (!HasAuthority() || !User) return;

    IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(User);
    if (!ASCInterface) return;

    UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
    if (!ASC) return;

    const float CurrentCredits = ASC->GetNumericAttribute(UVGPlayerAttributeSet::GetCreditsAttribute());
    ASC->SetNumericAttributeBase(UVGPlayerAttributeSet::GetCreditsAttribute(), CurrentCredits + CreditValue);

    UE_LOG(LogTemp, Log, TEXT("[NCCreditItem] %s 크레딧 획득: %d"), *User->GetName(), CreditValue);

    Destroy();
}

void ANCCreditItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANCCreditItemActor, CreditValue);
}