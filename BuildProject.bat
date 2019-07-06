@echo off

rem Input args: BuildName, Platform, BuildConfiguration, uproject

call "%~dp0ProjectPaths.bat"

rem Try to build using a project plugin first.
set BuildWorkerBat="%~dp0%PROJECT_PATH%/Plugins/UnrealGDK/SpatialGDK/Build/Scripts/BuildWorker.bat"
if exist %BuildWorkerBat% (
    goto :Build
)

rem If no project plugin exists, try to build using an engine plugin.
set BuildWorkerBat="%~dp0../../Engine/Plugins/UnrealGDK/SpatialGDK/Build/Scripts/BuildWorker.bat"
if not exist %BuildWorkerBat% (
    echo Cannot find BuildWorker.bat! Please check you have the SpatialGDK plugin installed.
    goto :Error
)

:Build
call %BuildWorkerBat% %GAME_NAME%Server Linux Development %GAME_NAME%.uproject || goto :Error
call %BuildWorkerBat% %GAME_NAME% Win64 Development %GAME_NAME%.uproject || goto :Error

pause
exit /b 0

:Error
echo Builds failed.
pause
exit /b 1
