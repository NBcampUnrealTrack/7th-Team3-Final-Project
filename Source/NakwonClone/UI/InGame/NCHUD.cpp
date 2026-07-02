#include "NCHud.h"
#include "AbilitySystemComponent.h"
#include "NCStaminaBar.h"
#include "Player/PlayerCharacter/NCBaseCharacter.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "UI/InGame/NCHPBar.h"
#include "Components/Image.h" //헌호수정

void UNCHud::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	ANCPlayerCharacter* Character =Cast<ANCPlayerCharacter>(PC->GetPawn());
	if (!Character)
	{
		return;
	}

	ASC = Character->GetAbilitySystemComponent();
	AttributeSet = Character->GetPlayerAttributeSet();
	if (!ASC || !AttributeSet)
	{
		return;
	}

	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &UNCHud::OnHPChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetStaminaAttribute()).AddUObject(this, &UNCHud::OnStaminaChanged);
	
	if (HPBarWidget)
	{
		HPBarWidget->UpdateHP(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	}
	if (StaminaBarWidget)
	{
		StaminaBarWidget->UpdateStaminaBar(AttributeSet->GetStamina(), AttributeSet->GetMaxStamina());
	}

	//헌호수정 - 시작 시 피 오버레이 숨김
	if (BloodOverlay)
	{
		CurrentBloodOpacity = 0.f;
		BloodOverlay->SetRenderOpacity(0.f);
	}
}

//헌호수정 - 매 프레임 피 오버레이 투명도를 목표값으로 부드럽게 보간
void UNCHud::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!BloodOverlay || !AttributeSet)
	{
		return;
	}

	// 저체력이면 baseline을 LowHealthOpacity로, 아니면 0으로
	const float Health = AttributeSet->GetHealth();
	const float Baseline = (Health <= LowHealthThreshold) ? LowHealthOpacity : 0.f;

	// 현재 투명도를 baseline까지 서서히 감소 (피격 순간엔 위로 확 튀어있음)
	if (CurrentBloodOpacity > Baseline)
	{
		CurrentBloodOpacity = FMath::FInterpConstantTo(CurrentBloodOpacity, Baseline, InDeltaTime, BloodFadeSpeed);
	}
	else
	{
		CurrentBloodOpacity = Baseline;
	}

	BloodOverlay->SetRenderOpacity(CurrentBloodOpacity);
}

void UNCHud::OnHPChanged(const FOnAttributeChangeData& Data)
{
	if (!HPBarWidget || !AttributeSet)
	{
		return;
	}

	HPBarWidget->UpdateHP(Data.NewValue, AttributeSet->GetMaxHealth());

	//헌호수정 - 피격(체력 감소) 순간 피 오버레이 확 올림 → NativeTick에서 서서히 페이드
	if (BloodOverlay && Data.NewValue < Data.OldValue)
	{
		CurrentBloodOpacity = HitFlashOpacity;
		BloodOverlay->SetRenderOpacity(CurrentBloodOpacity);
	}
}

void UNCHud::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning,
		TEXT("Stamina Changed : %f"),
		Data.NewValue);
	
	if (!StaminaBarWidget || !AttributeSet)
	{
		return;
	}

	StaminaBarWidget->UpdateStaminaBar(Data.NewValue, AttributeSet->GetMaxStamina());
}

