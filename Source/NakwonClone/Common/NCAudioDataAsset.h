#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NCAudioDataAsset.generated.h"

class USoundBase;

UCLASS(BlueprintType)
class NAKWONCLONE_API UNCAudioDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> StartupBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> LoadingBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> TitleBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> LobbyBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> MVPAmbientBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BGM")
	TObjectPtr<USoundBase> ClearBGM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ambient")
	TArray<TObjectPtr<USoundBase>> RandomAmbientSounds;
};