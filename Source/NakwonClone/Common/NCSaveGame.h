#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/NCInventoryBaseComponent.h"

#include "NCSaveGame.generated.h"

UCLASS()
class NAKWONCLONE_API UNCSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UNCSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	TArray<FInventorySlot> PlayerInventoryItems;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FEquipmentPreset> PlayerEquipmentPresets;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FInventorySlot> PlayerConsumableSlots;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	TArray<FInventorySlot> StashInventoryItems;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 UserIndex;
};
