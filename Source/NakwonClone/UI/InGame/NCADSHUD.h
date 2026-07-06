// 헌호수정 - 정조준(ADS) 시 화면 하단에 뜨는 HUD: HP/탄약/무기이름
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NCADSHUD.generated.h"

class UProgressBar;
class UTextBlock;
class ANCPlayerCharacter;

UCLASS()
class NAKWONCLONE_API UNCADSHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// 헌호수정 - 데이터 갱신 (캐릭터가 ADS 중 매 프레임 호출)
	void RefreshAll(ANCPlayerCharacter* Player);

protected:
	// 위젯 이름과 정확히 일치해야 바인딩됨
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HPProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponNameText;
};
