// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/PlayerData/NCWeaponData.h"
#include "NCGameInstance.generated.h"

UCLASS()
class NAKWONCLONE_API UNCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	//무기 DataTable - 에디터에서 BP_NCGameInstance에 연결
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UDataTable> WeaponDataTable;

	//WeaponID로 무기 데이터 조회 (어디서든 호출 가능)
	FNCWeaponData* GetWeaponData(FName WeaponID) const;
};
