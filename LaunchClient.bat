@echo off
call "%~dp0FindEngine.bat"
call "%~dp0ProjectPaths.bat"

:: Need to strip the path to engine from quotes, so that we can combine it with the path to editor executable.
set UNREAL_ENGINE_STRIPPED_FROM_QUOTES=%UNREAL_ENGINE:~1,-1%

"%UNREAL_ENGINE_STRIPPED_FROM_QUOTES%\Engine\Binaries\Win64\UE4Editor.exe" "%~dp0%PROJECT_PATH%\%GAME_NAME%.uproject" 127.0.0.1 -game -log -workerType UnrealClient -stdout -nowrite -unattended -nologtimes -nopause -noin -messaging -NoVerifyGC -windowed -ResX=800 -ResY=600
