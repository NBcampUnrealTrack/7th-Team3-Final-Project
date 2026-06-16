
#include "NCGameInstance.h"

#include "MoviePlayer.h"
#include "Widgets/Images/SThrobber.h"
#include "Fonts/SlateFontInfo.h"
#include "Widgets/Notifications/SProgressBar.h"

void UNCGameInstance::Init()
{
	Super::Init();
	RegisterLoadingScreenHandlers();
}

FNCWeaponData* UNCGameInstance::GetWeaponData(FName WeaponID) const
{
	if (!WeaponDataTable || WeaponID.IsNone()) return nullptr;

	return WeaponDataTable->FindRow<FNCWeaponData>(WeaponID, TEXT("GetWeaponData"));
}

#pragma region Loading Screen
void UNCGameInstance::RegisterLoadingScreenHandlers()
{
	// 델리게이트 바인딩(이벤트 발생 시 함수 호출 신청)
	// FCoreUObjectDelegates::PreLoadMap - 맵 로딩 직전에 엔진 전역에 브로드캐스트하는 이벤트
	FCoreUObjectDelegates::PreLoadMap.AddUObject(
		this,
		&UNCGameInstance::BeginLoadingScreen);
	
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&UNCGameInstance::EndLoadingScreen);
}

void UNCGameInstance::BeginLoadingScreen(const FString& MapName)
{
	// 배경 텍스처가 에디터에서 연결되어 있으면 브러시로 변환하는 로직
	if (LoadingBackgroundTexture)
	{
		// SlateBrush - 슬레이트(UE의 렌더링 시스템)가 이미지를 어떻게 그릴지 정의하는 구조체
		BackgroundBrush.SetResourceObject(LoadingBackgroundTexture);
		BackgroundBrush.ImageSize = FVector2D(1920.f, 1080.f);
		BackgroundBrush.DrawAs = ESlateBrushDrawType::Image; // 단순 이미지로 그리기
	}
	
	FLoadingScreenAttributes LoadingScreen; // 로딩 화면 설정을 담는 구조체
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true; // 로딩 끝나면 자동으로 로딩 화면 닫힘(false 시 수동)
	LoadingScreen.MinimumLoadingScreenDisplayTime = 2.0f;
	LoadingScreen.WidgetLoadingScreen = SNew(SOverlay)
	
	// Layer1 : Image
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SImage).Image(&BackgroundBrush)	
		]
	
	// Layer2 : Text
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 70.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("로딩중...")))
			.ColorAndOpacity(FLinearColor::White)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
		]
	
	// Layer3 : Loading Bar
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 50.f)
		[
			SNew(SBox).WidthOverride(300.f).HeightOverride(12.f)
			[
				SNew(SProgressBar)
			]
		];
	
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen); // 엔진의 MoviePlayer에 설정 전달 -> 로딩 화면 관리를 MoviePlayer에게 맡기는 로직
}

void UNCGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
	// 엔진이 자동으로 처리함(true)
}
#pragma endregion

