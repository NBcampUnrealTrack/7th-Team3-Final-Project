
#include "NCGameInstance.h"

#include "MoviePlayer.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Images/SThrobber.h"
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
	if (!MapName.Contains(TEXT("L_ShoppingMall")) && !MapName.Contains(TEXT("L_TitleAndLobby")) && !MapName.Contains(TEXT("L_Lobby")) && !MapName.Contains(TEXT("L_Parking")))
		return;

	if (MapName.Contains(TEXT("L_TitleAndLobby")) && !bHasLoadedOnce)
	{
		bHasLoadedOnce = true;
		return;
	}
	
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
	
		// Layer2 : Text + Spinner
		+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 20.f, 20.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(SCircularThrobber)
				.Radius(16.f)
				.Period(1.0f)
				.NumPieces(4)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Loading...")))
				.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.f))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			]
		];
	
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen); // 엔진의 MoviePlayer에 설정 전달 -> 로딩 화면 관리를 MoviePlayer에게 맡기는 로직
}

void UNCGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
	// 엔진이 자동으로 처리함(true)
}

#pragma endregion

#pragma region Preloading

void UNCGameInstance::StartPreloading()
{
	// FSoftObjectPath — 에셋을 직접 로딩하지 않고 경로만 참조하는 타입
	// (직접 참조하면 게임 시작 시 전부 로딩되어 버림 — 그걸 방지하기 위해 경로만 들고 있는 것)
	TArray<FSoftObjectPath> AssetsToPreload;

	// 무기 에셋 프리로드 — Internal_EquipWeapon의 LoadSynchronous() 블로킹 방지
	if (WeaponDataTable)
	{
		TArray<FNCWeaponData*> Rows;
		WeaponDataTable->GetAllRows<FNCWeaponData>(TEXT("Preload"), Rows);
		for (const FNCWeaponData* Row : Rows)
		{
			if (Row)
			{
				if (!Row->AttackMontage.IsNull())      AssetsToPreload.Add(Row->AttackMontage.ToSoftObjectPath());
				if (!Row->HeavyAttackMontage.IsNull()) AssetsToPreload.Add(Row->HeavyAttackMontage.ToSoftObjectPath());
				if (!Row->WeaponActorClass.IsNull())   AssetsToPreload.Add(Row->WeaponActorClass.ToSoftObjectPath());
			}
		}
	}

	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_Clothes01.SM_Merged_Clothes01")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_RackBoot02.SM_Merged_RackBoot02")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_RackBoot01.SM_Merged_RackBoot01")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_Clothes20.SM_Merged_Clothes20")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Electronics/SM_Merged_Phone08.SM_Merged_Phone08")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_Clothes07.SM_Merged_Clothes07")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_Clothes19.SM_Merged_Clothes19")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_RackBoot04.SM_Merged_RackBoot04")));
	AssetsToPreload.Add(FSoftObjectPath(TEXT("/Game/Asset/Map/ShoppingMall/Meshes/Interior/Clothes/SM_Merged_Clothes12.SM_Merged_Clothes12")));
	
	// UAssetManager — UE의 전역 에셋 관리자. FStreamableManager를 여기서 가져오는 게 권장 방식
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	
	// RequestAsyncLoad — 비동기로 에셋 로딩 시작
	// 백그라운드에서 로딩하므로 게임 흐름을 막지 않음
	// 완료되면 OnPreloadComplete 콜백 호출
	PreloadHandle = StreamableManager.RequestAsyncLoad(
		AssetsToPreload,
		FStreamableDelegate::CreateUObject(this, &UNCGameInstance::OnPreloadComplete));
	
	UE_LOG(LogTemp, Log, TEXT("프리로딩 시작 — %d개 에셋"), AssetsToPreload.Num());
}

void UNCGameInstance::OnPreloadComplete()
{
	UE_LOG(LogTemp, Log, TEXT("프리로딩 완료"));
}

#pragma endregion