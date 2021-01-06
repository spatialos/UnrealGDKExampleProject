param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, 
  [string] $unreal_engine_home = (Get-Item "$($PSScriptRoot)").parent.parent.parent.FullName, 
  [string] $auth_token = "YThkOTFlMzQtM2EzMi00MTM0LWEzOGQtNmM3YjQ0YzY4NzRkOjo1NWVmZGQxZi1hMmU2LTRiODItYWQ4Ny04OGJjNGY2NzUzYzU=",
  [string] $deployment_name = "ken_d"
)

. "$PSScriptRoot\common.ps1"
$game_project = "$exampleproject_home/Game/GDKShooter.uproject"

pushd "$exampleproject_home"
    Write-Host "Cloud deployment to connect to: $deployment_name"
    Write-Host "exampleproject_home: $exampleproject_home"
    Write-Host "build_home: $build_home"
    Write-Host "unreal_engine_home: $unreal_engine_home"
    
    $cmdline="127.0.0.1 -workerType UnrealClient -devauthToken $auth_token -deployment $deployment_name -linkProtocol Tcp"
    $argumentlist = @(`
        "-ScriptsForProject=$game_project", `
        "BuildCookRun", `
        "-nocompileeditor", `
        "-nop4", `
        "-project=$game_project", `
        "-cook", `
        "-stage", `
        "-archive", `
        "-archivedirectory=$($exampleproject_home)/cooked-android", `
        "-package", `
        "-clientconfig=Development", `
        "-ue4exe=$($unreal_engine_home)/Engine/Binaries/Win64/UE4Editor-Cmd.exe", `
        "-pak", `
        "-prereqs", `
        "-nodebuginfo", `
        "-targetplatform=Android", `
        "-cookflavor=DXT", `
        "-build", `
        "-utf8output", `
        "-cmdline=`"${cmdline}`""
    )
    $unreal_uat_path = "${unreal_engine_home}\Engine\Build\BatchFiles\RunUAT.bat"
    Write-Host "unreal_uat_path=$unreal_uat_path"
    Write-Host "argumentlist=$argumentlist"
    $build_server_proc = Start-Process -PassThru -NoNewWindow -FilePath $unreal_uat_path -ArgumentList $argumentlist
    $build_server_handle = $build_server_proc.Handle
    Wait-Process -InputObject $build_server_proc
    if ($build_server_proc.ExitCode -ne 0) {
        Write-Host "Failed to build Android Development Client. Error: $($build_server_proc.ExitCode)"
        Throw "Failed to build Android Development Client"
    }
popd
