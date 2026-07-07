@echo off
setlocal enabledelayedexpansion
chcp 65001

:: ============================
:: 프로젝트, 엔진 위치 등 빌드 관련 설정
:: ============================
set PROJECT_NAME=NakwonClone
set ENGINE_PATH="C:\UE_5.6_Custom\Engine"
set CONFIG=Development

set BUILD_PLATFORM=Win64
set PROJECT_PATH=%~dp0%PROJECT_NAME%.uproject
set UAT_PATH=%ENGINE_PATH%\Build\BatchFiles\RunUAT.bat
set BUILD_PATH=%ENGINE_PATH%\Build\BatchFiles\Build.bat

echo ============================
echo 임시 파일 제거(Binaries, Intermediate)

if exist "%~dp0Binaries" rd /s /q "%~dp0Binaries"
if exist "%~dp0Intermediate" rd /s /q "%~dp0Intermediate"

echo 빌드 시작: %PROJECT_NAME%
echo ============================

echo 에디터 빌드 중...
call %BUILD_PATH% %PROJECT_NAME%Editor %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex

if errorlevel 1 (
    echo [오류] 에디터 빌드 실패!
    pause
    exit /b !ERRORLEVEL!
)

echo 게임 빌드 중...
call %BUILD_PATH% %PROJECT_NAME% %BUILD_PLATFORM% %CONFIG% -project="%PROJECT_PATH%" -waitmutex

if errorlevel 1 (
    echo [오류] 게임 빌드 실패!
    pause
    exit /b !ERRORLEVEL!
)

echo 컨텐츠 쿠킹 및 패키징 중...
call %UAT_PATH% BuildCookRun ^
    -project="%PROJECT_PATH%" ^
    -noP4 ^
    -platform=%BUILD_PLATFORM% ^
    -targetplatform=%BUILD_PLATFORM% ^
    -clean ^
    -cook -allmaps -stage -pak -archive ^
    -archivedirectory="%~dp0BuildOutput"

if errorlevel 1 (
    echo 쿠킹 및 패키징 중 오류 발생! 코드=!ERRORLEVEL!
    pause
    exit /b !ERRORLEVEL!
)

echo ============================
echo 모든 작업 완료!
echo ============================
pause