
#include "NCGameInstance.h"

#include "MoviePlayer.h"
#include "Widgets/Images/SThrobber.h"

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

#pragma region Loading
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
	FLoadingScreenAttributes LoadingScreen; // 로딩 화면 설정을 담는 구조체
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true; // 로딩 끝나면 자동으로 로딩 화면 닫힘(false 시 수동)
	LoadingScreen.MinimumLoadingScreenDisplayTime = 2.0f; // 최소 로딩 화면 뜨는 시간
	LoadingScreen.WidgetLoadingScreen = SNew(SThrobber); // 로딩 화면에 표시할 slate 위젯 설정(SThrobber == UE 기본 로딩 위젯)
	// 이미지로 교체 예시 - LoadingScreen.WidgetLoadingScreen = SNew(SImage.Image(/* Slate Brush 넣기 */);
	// 영상(.mp4, .bk2) - LoadingScreen.MoviePaths.Add(TEXT("LoadingMovie")); // Content/Movies/ 폴더에 넣은 영상 이름
	
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen); // 엔진의 MoviePlayer에 설정 전달 -> 로딩 화면 관리를 MoviePlayer에게 맡기는 로직
}

void UNCGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{
	// 엔진이 자동으로 처리함(true)
}
#pragma endregion