// 헌호수정 - 백팩(등) 3D HUD: HP/탄약/소지품/무기를 표시
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NCBackpackHUD.generated.h"

class UProgressBar;
class UTextBlock;
class ANCPlayerCharacter;

UCLASS()
class NAKWONCLONE_API UNCBackpackHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// 헌호수정 - 캐릭터가 매 프레임 호출: 모든 정보 갱신
	void RefreshAll(ANCPlayerCharacter* Player);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// 위젯 이름과 정확히 일치해야 바인딩됨 (없어도 크래시 X)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SnackText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Weapon1Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Weapon2Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Weapon3Text;
};
