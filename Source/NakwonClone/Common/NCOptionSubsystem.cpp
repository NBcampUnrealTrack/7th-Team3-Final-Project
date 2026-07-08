#include "NCOptionSubsystem.h"
#include "NCOptionSaveGame.h"

#include "InputAction.h"
#include "EnhancedInputLibrary.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UNCOptionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MasterVolume = 1.f;
	SFXVolume = 1.f;
	BGMVolume = 1.f;
	VoiceVolume = 1.f;

	// 리바인드/저장 시 "기본값이랑 다른지" 비교할 기준점을 미리 스냅샷해둔다
	DefaultKeyMappings.Reset();
	if (RebindableMappingContext)
	{
		DefaultKeyMappings = RebindableMappingContext->GetMappings();
	}

	if (UWorld* World = GetCurrentWorld())
	{
		if (MasterSoundMix)
		{
			UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);
		}
	}

	LoadOptions();
}

void UNCOptionSubsystem::Deinitialize()
{
	if (UWorld* World = GetCurrentWorld())
	{
		if (MasterSoundMix)
		{
			UGameplayStatics::PopSoundMixModifier(World, MasterSoundMix);
		}
	}

	Super::Deinitialize();
}

UWorld* UNCOptionSubsystem::GetCurrentWorld() const
{
	const ULocalPlayer* LP = GetLocalPlayer();
	return LP ? LP->GetWorld() : nullptr;
}

void UNCOptionSubsystem::SetMasterVolume(float NewVolume)
{
	MasterVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	ApplyVolume();
}

void UNCOptionSubsystem::SetSFXVolume(float NewVolume)
{
	SFXVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	ApplyVolume();
}

void UNCOptionSubsystem::SetBGMVolume(float NewVolume)
{
	BGMVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	ApplyVolume();
}

void UNCOptionSubsystem::SetVoiceVolume(float NewVolume)
{
	VoiceVolume = FMath::Clamp(NewVolume, 0.f, 1.f);
	ApplyVolume();
}

void UNCOptionSubsystem::ApplyVolume()
{
	UWorld* World = GetCurrentWorld();
	if (!World || !MasterSoundMix)
	{
		return;
	}

	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, MasterSoundClass, MasterVolume, 1.f, 0.f, true);
	}
	if (SFXSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, SFXSoundClass, SFXVolume, 1.f, 0.f, true);
	}
	if (BGMSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, BGMSoundClass, BGMVolume, 1.f, 0.f, true);
	}
	if (VoiceSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, VoiceSoundClass, VoiceVolume, 1.f, 0.f, true);
	}
}

bool UNCOptionSubsystem::RebindAction(UInputAction* Action, FKey NewKey)
{
	if (!Action || !RebindableMappingContext)
	{
		return false;
	}

	RebindableMappingContext->MapKey(Action, NewKey);
	UEnhancedInputLibrary::RequestRebuildControlMappingsUsingContext(RebindableMappingContext, /*bForceImmediately=*/true);

	return true;
}

FKey UNCOptionSubsystem::GetKeyForAction(UInputAction* Action) const
{
	if (RebindableMappingContext)
	{
		for (const FEnhancedActionKeyMapping& Mapping : RebindableMappingContext->GetMappings())
		{
			if (Mapping.Action == Action)
			{
				return Mapping.Key;
			}
		}
	}
	return EKeys::Invalid;
}

void UNCOptionSubsystem::ResetKeyBindingsToDefault()
{
	if (!RebindableMappingContext)
	{
		return;
	}

	for (const FEnhancedActionKeyMapping& DefaultMapping : DefaultKeyMappings)
	{
		if (DefaultMapping.Action)
		{
			RebindableMappingContext->MapKey(DefaultMapping.Action, DefaultMapping.Key);
		}
	}

	UEnhancedInputLibrary::RequestRebuildControlMappingsUsingContext(RebindableMappingContext, /*bForceImmediately=*/true);
}

void UNCOptionSubsystem::SaveOptions()
{
	UNCOptionSaveGame* SaveObj = Cast<UNCOptionSaveGame>(UGameplayStatics::CreateSaveGameObject(UNCOptionSaveGame::StaticClass()));
	if (!SaveObj)
	{
		return;
	}

	SaveObj->MasterVolume = MasterVolume;
	SaveObj->SFXVolume = SFXVolume;
	SaveObj->BGMVolume = BGMVolume;
	SaveObj->VoiceVolume = VoiceVolume;

	SaveObj->KeyBindOverrides.Empty();
	if (RebindableMappingContext)
	{
		for (const FEnhancedActionKeyMapping& Mapping : RebindableMappingContext->GetMappings())
		{
			if (!Mapping.Action)
			{
				continue;
			}

			const FEnhancedActionKeyMapping* DefaultMapping = DefaultKeyMappings.FindByPredicate(
				[&Mapping](const FEnhancedActionKeyMapping& Candidate) { return Candidate.Action == Mapping.Action; });

			// 기본값과 다른 키만 저장 - 나머지는 코드/에셋 쪽 기본값을 그대로 따라간다
			if (!DefaultMapping || DefaultMapping->Key != Mapping.Key)
			{
				UInputAction* MutableAction = const_cast<UInputAction*>(Mapping.Action.Get());
				SaveObj->KeyBindOverrides.Add(TSoftObjectPtr<UInputAction>(MutableAction), Mapping.Key);
			}
		}
	}

	UGameplayStatics::SaveGameToSlot(SaveObj, SaveObj->SaveSlotName, SaveObj->UserIndex);
}

void UNCOptionSubsystem::LoadOptions()
{
	const FString SlotName = UNCOptionSaveGame::GetDefaultSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		ApplyVolume();
		return;
	}

	UNCOptionSaveGame* Loaded = Cast<UNCOptionSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Loaded)
	{
		ApplyVolume();
		return;
	}

	MasterVolume = Loaded->MasterVolume;
	SFXVolume = Loaded->SFXVolume;
	BGMVolume = Loaded->BGMVolume;
	VoiceVolume = Loaded->VoiceVolume;
	ApplyVolume();

	for (const TPair<TSoftObjectPtr<UInputAction>, FKey>& Pair : Loaded->KeyBindOverrides)
	{
		if (UInputAction* Action = Pair.Key.LoadSynchronous())
		{
			RebindAction(Action, Pair.Value);
		}
	}
}
