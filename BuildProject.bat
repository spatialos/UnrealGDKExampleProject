@echo off
call "%~dp0ProjectPaths.bat"
:: call %~dp0%PROJECT_PATH%\"Plugins/UnrealGDK/SpatialGDK/Build/Scripts/BuildWorker.bat" %GAME_NAME%Server Linux Development %GAME_NAME%.uproject || goto :error
call %~dp0%PROJECT_PATH%\"Plugins/UnrealGDK/SpatialGDK/Build/Scripts/BuildWorker.bat" %GAME_NAME% Win64 Development %GAME_NAME%.uproject || goto :error
echo All builds succeeded.

pause
exit /b 0

:error
echo Builds failed.
pause
exit /b 1
