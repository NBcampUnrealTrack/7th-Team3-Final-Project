#include "Common/NCAudioSubsystem.h"
#include "Common/NCAudioDataAsset.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UNCAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BGMComponent = nullptr;
	RandomAmbientComponent = nullptr;
	CurrentState = ENCAudioState::None;

	AudioData = LoadObject<UNCAudioDataAsset>(
		nullptr,
		TEXT("/Game/Sound/DA_Audio.DA_Audio")
	);
}

void UNCAudioSubsystem::Deinitialize()
{
	StopAllAudio();
	Super::Deinitialize();
}

void UNCAudioSubsystem::PlayStartup()
{
	if (!AudioData) return;
	FadeToBGM(AudioData->StartupBGM, ENCAudioState::Startup);
}

void UNCAudioSubsystem::PlayLoading()
{
	if (!AudioData) return;

	StopRandomAmbient();
	FadeToBGM(AudioData->LoadingBGM, ENCAudioState::Loading);
}

void UNCAudioSubsystem::PlayTitle()
{
	if (!AudioData) return;

	StopRandomAmbient();
	FadeToBGM(AudioData->TitleBGM, ENCAudioState::Title);
}

void UNCAudioSubsystem::PlayLobby()
{
	if (!AudioData) return;

	StopRandomAmbient();
	FadeToBGM(AudioData->LobbyBGM, ENCAudioState::Lobby);
}

void UNCAudioSubsystem::PlayMVP()
{
	if (!AudioData) return;

	FadeToBGM(AudioData->MVPAmbientBGM, ENCAudioState::MVP);
	StartRandomAmbient();
}

void UNCAudioSubsystem::PlayClear()
{
	if (!AudioData) return;

	StopRandomAmbient();
	FadeToBGM(AudioData->ClearBGM, ENCAudioState::Clear);
}

void UNCAudioSubsystem::FadeToBGM(USoundBase* NewSound, ENCAudioState NewState)
{
	if (!NewSound)
	{
		return;
	}

	if (CurrentState == NewState && BGMComponent && BGMComponent->IsPlaying())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!BGMComponent)
	{
		BGMComponent = UGameplayStatics::CreateSound2D(World, NewSound, BGMVolume);
		if (!BGMComponent)
		{
			return;
		}

		BGMComponent->bAutoDestroy = false;
		BGMComponent->FadeIn(FadeTime, BGMVolume);
		CurrentState = NewState;
		return;
	}

	BGMComponent->FadeOut(FadeTime, 0.0f);
	BGMComponent->SetSound(NewSound);
	BGMComponent->FadeIn(FadeTime, BGMVolume);

	CurrentState = NewState;
}

void UNCAudioSubsystem::StartRandomAmbient()
{
	if (!AudioData || AudioData->RandomAmbientSounds.Num() <= 0)
	{
		return;
	}

	ScheduleNextRandomAmbient();
}

void UNCAudioSubsystem::StopRandomAmbient()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(RandomAmbientTimerHandle);
	}

	if (RandomAmbientComponent)
	{
		RandomAmbientComponent->Stop();
	}
}

void UNCAudioSubsystem::ScheduleNextRandomAmbient()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float NextTime = FMath::RandRange(RandomAmbientMinTime, RandomAmbientMaxTime);

	World->GetTimerManager().SetTimer(
		RandomAmbientTimerHandle,
		this,
		&UNCAudioSubsystem::PlayRandomAmbientOnce,
		NextTime,
		false
	);
}

void UNCAudioSubsystem::PlayRandomAmbientOnce()
{
	if (!AudioData || AudioData->RandomAmbientSounds.Num() <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 Index = FMath::RandRange(0, AudioData->RandomAmbientSounds.Num() - 1);
	USoundBase* SelectedSound = AudioData->RandomAmbientSounds[Index];

	if (SelectedSound)
	{
		RandomAmbientComponent = UGameplayStatics::CreateSound2D(
			World,
			SelectedSound,
			RandomAmbientVolume
		);

		if (RandomAmbientComponent)
		{
			RandomAmbientComponent->Play();
		}
	}

	ScheduleNextRandomAmbient();
}

void UNCAudioSubsystem::StopAllAudio()
{
	StopRandomAmbient();

	if (BGMComponent)
	{
		BGMComponent->FadeOut(FadeTime, 0.0f);
	}

	CurrentState = ENCAudioState::None;
}