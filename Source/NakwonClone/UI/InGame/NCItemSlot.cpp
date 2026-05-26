#include "NCItemSlot.h"

void UNCItemSlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 캐릭터 델리게이트 연결
}

void UNCItemSlot::OnLeftSlotImageChanged(UTexture2D* ItemIcon)
{
	if (LeftSlotImage)
	{
		LeftSlotImage->SetBrushFromTexture(ItemIcon);
	}
	
	// 무기 데이터 연동 후 주석 제거
	
	// if (TwoHandWeapon)
	// {
	// 	RightSizeBox->SetVisibility(ESlateVisibility::Collapsed);
	// 	LeftSizeBox->SetWidthOverride(300.f);
	// }
	// else
	// {
	// 	RightSizeBox->SetVisibility(ESlateVisibility::Visible);
	// 	LeftSizeBox->SetWidthOverride(150.f);
	// }
}

void UNCItemSlot::OnRightSlotImageChanged(UTexture2D* ItemIcon)
{
	if (RightSlotImage)
	{
		RightSlotImage->SetBrushFromTexture(ItemIcon);
	}
}

void UNCItemSlot::UpdateSlotHighlight()
{
	// 키 입력 바인딩 후 주석 제거

	// if (1번 키입력)
	// {
	// 	FirstItemSlotNumber->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));
	// 	FirstItemSlotImage->SetBrushTintColor(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.3f)));
	// 	
	// 	SecondItemSlotNumber->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f)));
	// 	SecondItemSlotImage->SetBrushTintColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.3f)));
	// }
	// else
	// {
	// 	FirstItemSlotNumber->SetColorAndOpacity(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f)));
	// 	FirstItemSlotImage->SetBrushTintColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.3f)));
	// 	
	// 	SecondItemSlotNumber->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)));
	// 	SecondItemSlotImage->SetBrushTintColor(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.3f)));
	// }
}