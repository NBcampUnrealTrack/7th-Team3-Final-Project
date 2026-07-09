#include "NCHud.h"
#include "AbilitySystemComponent.h"
#include "NCStaminaBar.h"
#include "Player/PlayerCharacter/NCBaseCharacter.h"
#include "GAS/AttributeSet/VGPlayerAttributeSet.h"
#include "Player/PlayerCharacter/NCPlayerCharacter.h"
#include "UI/InGame/NCHPBar.h"
#include "Components/Image.h" //헌호수정
#include "Components/AudioComponent.h" //헌호수정
#include "Kismet/GameplayStatics.h" //헌호수정
#include "Sound/SoundBase.h" //헌호수정
#include "TimerManager.h" //헌호수정

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
	const bool bLowHealth = (Health <= LowHealthThreshold);

	//헌호수정 - 저체력 사운드 시작/정지 처리
	if (bLowHealth && !bLowHealthActive)
	{
		StartLowHealthSound();
	}
	else if (!bLowHealth && bLowHealthActive)
	{
		StopLowHealthSound();
	}

	//헌호수정 - 저체력 중이면 체력에 따라 심박 피치 갱신
	if (bLowHealthActive)
	{
		UpdateHeartbeatPitch(Health);
	}

	const float Baseline = bLowHealth ? LowHealthOpacity : 0.f;

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
}

//헌호수정 - 저체력 진입: 심장박동 루프 시작 + 숨소리 타이머 시작
void UNCHud::StartLowHealthSound()
{
	bLowHealthActive = true;

	// 심장박동 루프 재생 (2D UI 사운드)
	if (HeartbeatSound)
	{
		HeartbeatAudioComp = UGameplayStatics::SpawnSound2D(this, HeartbeatSound, 1.f, HeartbeatMinPitch);
	}

	// 숨소리 첫 재생 예약
	if (BreathSound)
	{
		PlayBreathSound();
	}
}

//헌호수정 - 저체력 해제: 심장박동 정지 + 숨소리 타이머 해제
void UNCHud::StopLowHealthSound()
{
	bLowHealthActive = false;

	if (HeartbeatAudioComp)
	{
		HeartbeatAudioComp->Stop();
		HeartbeatAudioComp = nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BreathTimerHandle);
	}
}

//헌호수정 - 체력에 따라 심박 피치 갱신 (체력 낮을수록 빠르게)
void UNCHud::UpdateHeartbeatPitch(float Health)
{
	if (!HeartbeatAudioComp)
	{
		return;
	}

	// 체력 [Threshold~0] → Alpha [0~1] (0에 가까울수록 1)
	const float Alpha = FMath::Clamp(1.f - (Health / FMath::Max(LowHealthThreshold, 1.f)), 0.f, 1.f);
	const float Pitch = FMath::Lerp(HeartbeatMinPitch, HeartbeatMaxPitch, Alpha);
	HeartbeatAudioComp->SetPitchMultiplier(Pitch);
}

//헌호수정 - 숨소리 1회 재생 후 랜덤 간격으로 다음 재생 예약
void UNCHud::PlayBreathSound()
{
	if (!bLowHealthActive || !BreathSound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(this, BreathSound);

	// 다음 숨소리 예약 (랜덤 간격)
	if (UWorld* World = GetWorld())
	{
		const float NextInterval = FMath::FRandRange(BreathIntervalMin, BreathIntervalMax);
		World->GetTimerManager().SetTimer(BreathTimerHandle, this, &UNCHud::PlayBreathSound, NextInterval, false);
	}
}

