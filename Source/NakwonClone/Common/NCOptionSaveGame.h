#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "InputCoreTypes.h"
#include "NCOptionSaveGame.generated.h"

class UInputAction;

// 옵션(볼륨/키바인딩) 저장 전용 SaveGame.
// 인벤토리 저장용 UNCSaveGame과는 별개 슬롯을 사용한다 - 옵션은 PlayerController/레벨과 무관하게
// 타이틀/로비/인게임 어디서든 동일하게 읽고 쓸 수 있어야 하기 때문.
UCLASS()
class NAKWONCLONE_API UNCOptionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UNCOptionSaveGame();

	static FString GetDefaultSlotName() { return TEXT("NCOptionSlot"); }

	UPROPERTY(VisibleAnywhere, Category = "Option")
	float MasterVolume = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Option")
	float SFXVolume = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Option")
	float BGMVolume = 1.f;

	UPROPERTY(VisibleAnywhere, Category = "Option")
	float VoiceVolume = 1.f;

	// 기본값과 다르게 리바인드된 키만 저장 (SoftObjectPtr라 에셋을 미리 로드하지 않아도 됨)
	UPROPERTY(VisibleAnywhere, Category = "Option")
	TMap<TSoftObjectPtr<UInputAction>, FKey> KeyBindOverrides;

	UPROPERTY(VisibleAnywhere, Category = "Option")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "Option")
	int32 UserIndex;
};
