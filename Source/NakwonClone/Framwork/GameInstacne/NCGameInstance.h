// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/PlayerData/NCWeaponData.h"
#include "Styling/SlateBrush.h"
#include "NCGameInstance.generated.h"

UCLASS()
class NAKWONCLONE_API UNCGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	//무기 DataTable - 에디터에서 BP_NCGameInstance에 연결
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UDataTable> WeaponDataTable;

	//WeaponID로 무기 데이터 조회 (어디서든 호출 가능)
	FNCWeaponData* GetWeaponData(FName WeaponID) const;
	
#pragma region Loading Screen 띄우기	
private:
	void RegisterLoadingScreenHandlers(); // 로딩 화면 등록
	void BeginLoadingScreen(const FString& MapName); // 로딩 화면 시작 콜백(레벨 전환 시 자동 호출)
	void EndLoadingScreen(UWorld* InLoadedWorld); // 로딩 화면 종료 콜백(자동 호출)
	
#pragma endregion
	
#pragma region Loading Screen 이미지 추가
public:
	// 에디터에서 배경 이미지를 연결할 프로퍼티
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UTexture2D> LoadingBackgroundTexture;
	
private:
	FSlateBrush BackgroundBrush; // slate 브러시를 멤버변수로 보관(BegineLoadingScreen 호출 시 생성)	
#pragma endregion
};
