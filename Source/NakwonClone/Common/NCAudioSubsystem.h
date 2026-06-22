#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NCAudioSubsystem.generated.h"

class USoundBase;
class UAudioComponent;
class UNCAudioDataAsset;

UENUM(BlueprintType)
enum class ENCAudioState : uint8
{
	None,
	Startup,
	Loading,
	Title,
	Lobby,
	MVP,
	Clear
};

UCLASS()
class NAKWONCLONE_API UNCAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void PlayStartup();

	UFUNCTION(BlueprintCallable)
	void PlayLoading();

	UFUNCTION(BlueprintCallable)
	void PlayTitle();

	UFUNCTION(BlueprintCallable)
	void PlayLobby();

	UFUNCTION(BlueprintCallable)
	void PlayMVP();

	UFUNCTION(BlueprintCallable)
	void PlayClear();

	UFUNCTION(BlueprintCallable)
	void StopAllAudio();

	UFUNCTION(BlueprintCallable)
	void StartRandomAmbient();

	UFUNCTION(BlueprintCallable)
	void StopRandomAmbient();

protected:
	void FadeToBGM(USoundBase* NewSound, ENCAudioState NewState);
	void PlayRandomAmbientOnce();
	void ScheduleNextRandomAmbient();

protected:
	UPROPERTY()
	TObjectPtr<UNCAudioDataAsset> AudioData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Random Ambient")
	float RandomAmbientMinTime = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Random Ambient")
	float RandomAmbientMaxTime = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Fade")
	float FadeTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Volume")
	float BGMVolume = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio|Volume")
	float RandomAmbientVolume = 1.0f;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> RandomAmbientComponent;

	FTimerHandle RandomAmbientTimerHandle;

	ENCAudioState CurrentState = ENCAudioState::None;
};