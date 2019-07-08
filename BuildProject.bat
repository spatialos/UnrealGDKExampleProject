@echo off

call "%~dp0ProjectPaths.bat"

setlocal EnableDelayedExpansion

set GDKDirectory=""

rem If a project plugin exists. Use this for building.
if exist "%~dp0\%PROJECT_PATH%\Plugins\UnrealGDK" (
    set GDKDirectory="%~dp0\%PROJECT_PATH%\Plugins\UnrealGDK\"
    goto :BuildWorkers
)

rem If there is no project plugin. Find the engine plugin.

rem Get the Unreal Engine used by this project by querying the registry for the engine association found in the .uproject.
set UNREAL_ENGINE=""
set UPROJECT=""

rem First find the .uproject
for /f "delims=" %%A in (' powershell -Command "Get-ChildItem %~dp0 -Depth 1 -Filter *.uproject -File | %% {$_.FullName}" ') do set UPROJECT="%%A"

rem If the regex failed then it will return a string containing only a space.
if %UPROJECT%=="" (
    echo Error: Could not find uproject. Please make sure you have passed in the project directory correctly.
    pause
    exit /b 1
)

echo Using uproject: %UPROJECT%

rem Get the Engine association from the uproject.
for /f "delims=" %%A in (' powershell -Command "(Get-Content %UPROJECT% | ConvertFrom-Json).EngineAssociation" ') do set ENGINE_ASSOCIATION="%%A"

echo Engine association for uproject is: %ENGINE_ASSOCIATION%

rem If the engine association is a path then use this.
if exist "%ENGINE_ASSOCIATION%" (
    set UNREAL_ENGINE=%ENGINE_ASSOCIATION%
)

rem Try and use the engine association as a key in the registry to get the path to Unreal.
if %UNREAL_ENGINE%=="" (
    if not "%ENGINE_ASSOCIATION%"=="" (
        rem Query the registry for the path to the Unreal Engine using the engine associtation.
        for /f "usebackq tokens=3*" %%A in (`reg query "HKCU\Software\Epic Games\Unreal Engine\Builds" /v %ENGINE_ASSOCIATION%`) do (
            set UNREAL_ENGINE="%%A"
        )
    )
)

rem If there was no engine association then we need to climb the directory path of the project to find the Engine.
if %UNREAL_ENGINE%=="" (
    pushd "%~dp0"

    :climb_parent_directory
    if exist Engine (
        rem Check for the Build.version file to be sure we have found a correct Engine folder.
        if exist "Engine\Build\Build.version" (
            set UNREAL_ENGINE="!cd!"
        )
    ) else (
        rem This checks if we are in a root directory. If so we cannot check any higher and so should error out.
        if "%cd:~3,1%"=="" (
            echo Error: Could not find Unreal Engine folder. Please set a project association or ensure your game project is within an Unreal Engine folder.
            pause
            exit /b 1
        )
        cd ..
        goto :climb_parent_directory
    )

    popd
)

if %UNREAL_ENGINE%=="" (
    echo Error: Could not find the Unreal Engine. Please associate your '.uproject' with an engine version or ensure this game project is nested within an engine build.
    pause
    exit /b 1
)

rem Make the relative path absolute
pushd "%~dp0\%PROJECT_PATH%"
cd %UNREAL_ENGINE%

echo Using Unreal Engine at: %cd%

set GDKDirectory="%cd%\Engine\Plugins\UnrealGDK"
popd

:BuildWorkers
echo Building worker with GDK located at %GDKDirectory%

call %GDKDirectory%\SpatialGDK\Build\Scripts\BuildWorker.bat %GAME_NAME%Server Linux Development "%~dp0\%PROJECT_PATH%\%GAME_NAME%.uproject" || goto :error
call %GDKDirectory%\SpatialGDK\Build\Scripts\BuildWorker.bat %GAME_NAME% Win64 Development "%~dp0\%PROJECT_PATH%\%GAME_NAME%.uproject" || goto :error
echo All builds succeeded.

pause
exit /b 0

:error
echo Builds failed.
pause
exit /b 1
