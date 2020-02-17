@echo off
call "%~dp0FindEngine.bat"
call "%~dp0ProjectPaths.bat"
"%UNREAL_ENGINE:"=%\Engine\Binaries\Win64\UE4Editor.exe" "%~dp0%PROJECT_PATH%\%GAME_NAME%.uproject" Deployments -game -log -workerType UnrealClient -stdout -nowrite -unattended -nologtimes -nopause -noin -messaging -NoVerifyGC -windowed -ResX=800 -ResY=600
