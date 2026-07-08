#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "NCOptionSubsystem.generated.h"

class UInputAction;
class USoundMix;
class USoundClass;

// 옵션(볼륨/키바인딩) 값을 소유하고 저장/적용하는 LocalPlayerSubsystem.
//
// ULocalPlayerSubsystem은 LocalPlayer에 붙어있어서 타이틀/로비/인게임처럼
// 레벨마다 PlayerController 클래스(BP_NCPlayerController, TestController 등)가 달라도
// 항상 동일하게 존재한다. WB_Option은 더 이상 "Get Owning Player Controller -> Cast To
// BP_NCPlayerController"로 OwnerController를 구해서 볼륨/키바인딩을 만지면 안 되고,
// 이 서브시스템을 통해서만 만져야 한다 (Get Owning Player -> Get Local Player ->
// Get Subsystem(NCOptionSubsystem)).
//
// 화면(해상도/전체화면) 설정은 엔진 내장 UGameUserSettings가 이미 PlayerController와
// 무관하게 전역으로 동작하므로 이 서브시스템에서 다루지 않는다 - WB_Option에서
// "Get Game User Settings" 노드를 직접 써서 SetScreenResolution / ApplyResolutionSettings /
// SaveSettings를 호출하면 된다.
UCLASS(Blueprintable)
class NAKWONCLONE_API UNCOptionSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- Volume ----
	UFUNCTION(BlueprintCallable, Category = "Option|Sound")
	void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Option|Sound")
	void SetSFXVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Option|Sound")
	void SetBGMVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Option|Sound")
	void SetVoiceVolume(float NewVolume);

	UFUNCTION(BlueprintPure, Category = "Option|Sound")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "Option|Sound")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintPure, Category = "Option|Sound")
	float GetBGMVolume() const { return BGMVolume; }

	UFUNCTION(BlueprintPure, Category = "Option|Sound")
	float GetVoiceVolume() const { return VoiceVolume; }

	// ---- Key binding ----
	// RebindableMappingContext 안에서 Action을 NewKey로 즉시 재매핑한다 (원본 IMC 에셋은 건드리지 않고
	// 런타임에 로드된 인스턴스만 바뀐다).
	UFUNCTION(BlueprintCallable, Category = "Option|Input")
	bool RebindAction(UInputAction* Action, FKey NewKey);

	UFUNCTION(BlueprintPure, Category = "Option|Input")
	FKey GetKeyForAction(UInputAction* Action) const;

	UFUNCTION(BlueprintCallable, Category = "Option|Input")
	void ResetKeyBindingsToDefault();

	// ---- Persistence ----
	UFUNCTION(BlueprintCallable, Category = "Option")
	void SaveOptions();

	UFUNCTION(BlueprintCallable, Category = "Option")
	void LoadOptions();

protected:
	void ApplyVolume();
	UWorld* GetCurrentWorld() const;

	// 팀에서 BP_NCOptionSubsystem 같은 블루프린트 자식 클래스를 만들어 여기에 실제 SoundMix/
	// SoundClass/MappingContext 에셋을 클래스 디폴트에서 연결해서 쓰는 걸 권장 (C++만으로는
	// 에디터에서 에셋을 드래그해 넣을 UI가 없음).
	UPROPERTY(EditDefaultsOnly, Category = "Option|Sound")
	TObjectPtr<USoundMix> MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Option|Sound")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Option|Sound")
	TObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Option|Sound")
	TObjectPtr<USoundClass> BGMSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Option|Sound")
	TObjectPtr<USoundClass> VoiceSoundClass;

	// 리바인드 가능한 액션들이 들어있는 Input Mapping Context.
	// BP_NCPlayerController의 DefaultMappingContext와 "같은 에셋"을 지정해야
	// 여기서 MapKey()로 바꾼 게 실제 게임플레이 입력에도 반영된다.
	UPROPERTY(EditDefaultsOnly, Category = "Option|Input")
	TObjectPtr<UInputMappingContext> RebindableMappingContext;

	UPROPERTY(BlueprintReadOnly, Category = "Option|Sound")
	float MasterVolume = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Option|Sound")
	float SFXVolume = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Option|Sound")
	float BGMVolume = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Option|Sound")
	float VoiceVolume = 1.f;

	// Initialize() 시점의 RebindableMappingContext 매핑을 그대로 스냅샷 (기본값 복원/저장 비교용)
	UPROPERTY()
	TArray<FEnhancedActionKeyMapping> DefaultKeyMappings;
};
