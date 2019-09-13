param(
  [string] $game_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $gdk_repo = "git@github.com:spatialos/UnrealGDK.git",
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
  [string] $gdk_branch_name = "master",
  [string] $deployment_launch_configuration = "one_worker_test.json",
  [string] $deployment_snapshot_path = "snapshots/FPS-Start_Small.snapshot",
  [string] $deployment_cluster_region = "eu",
  [string] $project_name = "demo"
)

. "$PSScriptRoot\common.ps1"

$gdk_home = "$game_home\Game\Plugins\UnrealGDK"

pushd "$game_home"
    Start-Event "clone-gdk-plugin" "build-unreal-gdk-example-project-:windows:"
        pushd "Game"
            New-Item -Name "Plugins" -ItemType Directory -Force
            pushd "Plugins"
            Start-Process -Wait -PassThru -NoNewWindow -FilePath "git" -ArgumentList @(`
                "clone", `
                "$gdk_repo", `
                "--depth 1", `
                "-b $gdk_branch_name"
            )
            popd
        popd
    Finish-Event "clone-gdk-plugin" "build-unreal-gdk-example-project-:windows:"

    Start-Event "get-gdk-head-commit" "build-unreal-gdk-example-project-:windows:"
        pushd $gdk_home
            # Get the short commit hash of this gdk build for later use in assembly name
            $gdk_commit_hash = (git rev-parse HEAD).Substring(0,7)
            Write-Log "GDK at commit: $gdk_commit_hash on branch $gdk_branch_name"
        popd
    Finish-Event "get-gdk-head-commit" "build-unreal-gdk-example-project-:windows:"

    Start-Event "set-up-gdk-plugin" "build-unreal-gdk-example-project-:windows:"
        pushd $gdk_home
            # Set the required variables for the GDK's setup script to use
            $msbuild_exe = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe"

            # Invoke the GDK's setup script
            &"$($gdk_home)\ci\setup-gdk.ps1"
        popd
    Finish-Event "set-up-gdk-plugin" "build-unreal-gdk-example-project-:windows:"

    # Fetch the version of Unreal Engine we need from the GDK
    pushd "$($gdk_home)/ci"
        $unreal_version = Get-Content -Path "unreal-engine.version" -Raw
        Write-Log "Using Unreal Engine version: $unreal_version"
    popd

    Start-Event "download-unreal-engine" "build-unreal-gdk-example-project-:windows:"
        ## Create an UnrealEngine directory if it doesn't already exist
        $engine_directory = "UnrealEngine"
        New-Item -Name $engine_directory -ItemType Directory -Force

        pushd $engine_directory
            Write-Log "Downloading the Unreal Engine artifacts from GCS"
            $gcs_unreal_location = "$($unreal_version).zip"

            $gsu_proc = Start-Process -Wait -PassThru -NoNewWindow "gsutil" -ArgumentList @(`
                "cp", `
                "gs://$($gcs_publish_bucket)/$($gcs_unreal_location)", `
                "$($unreal_version).zip" `
            )
            if ($gsu_proc.ExitCode -ne 0) {
                Write-Log "Failed to download Engine artifacts. Error: $($gsu_proc.ExitCode)"
                Throw "Failed to download Engine artifacts"
            }

            Write-Log "Unzipping Unreal Engine"
            $zip_proc = Start-Process -Wait -PassThru -NoNewWindow "7z" -ArgumentList @(`
            "x", `
            "$($unreal_version).zip" `
            )
            if ($zip_proc.ExitCode -ne 0) {
                Write-Log "Failed to unzip Unreal Engine. Error: $($zip_proc.ExitCode)"
                Throw "Failed to unzip Unreal Engine."
            }
        popd
    Finish-Event "download-unreal-engine" "build-unreal-gdk-example-project-:windows:"

    Start-Event "associate-uproject-with-engine" "build-unreal-gdk-example-project-:windows:"
        pushd $engine_directory
            $unreal_version_selector_path = "$unreal_version\Engine\Binaries\Win64\UnrealVersionSelector-Win64-Shipping.exe"

            $find_engine_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath $unreal_version_selector_path -ArgumentList @(`
                "-switchversionsilent", `
                "$game_home\Game\GDKShooter.uproject", `
                "$engine_directory\$unreal_version"
            )
      
            if ($find_engine_process.ExitCode -ne 0) {
                Write-Log "Failed to set Unreal Engine association for the project. Error: $($find_engine_process.ExitCode)"
                Throw "Failed to set Engine association"
            }
        popd
    Finish-Event "associate-uproject-with-engine" "build-unreal-gdk-example-project-:windows:"

    # Set LINUX_MULTIARCH_ROOT and then reload it for this script
    $clang_path = "$($unreal_path)\ClangToolchain\"
    [Environment]::SetEnvironmentVariable("LINUX_MULTIARCH_ROOT", $clang_path, "Machine")
    $env:LINUX_MULTIARCH_ROOT = [System.Environment]::GetEnvironmentVariable("LINUX_MULTIARCH_ROOT", "Machine")

    Start-Event "install-unreal-engine-prerequisites" "build-unreal-gdk-example-project-:windows:"
        # This runs an opaque exe downloaded in the previous step that does *some stuff* that UE needs to occur.
        # Trapping error codes on this is tricky, because it doesn't always return 0 on success, and frankly, we just don't know what it _will_ return.
        Start-Process -Wait -PassThru -NoNewWindow -FilePath "$($unreal_path)\Engine\Extras\Redist\en-us\UE4PrereqSetup_x64.exe" -ArgumentList @(`
            "/quiet" `
        )
    Finish-Event "install-unreal-engine-prerequisites" "build-unreal-gdk-example-project-:windows:"

    $build_script_path = "$($gdk_home)\SpatialGDK\Build\Scripts\BuildWorker.bat"

    Start-Event "build-editor" "build-unreal-gdk-example-project-:windows:"
        # Build the project editor to allow the snapshot and schema commandlet to run
        $build_editor_proc = Start-Process -PassThru -NoNewWindow -FilePath $build_script_path -ArgumentList @(`
            "GDKShooterEditor", `
            "Win64", `
            "Development", `
            "GDKShooter.uproject"
        )

        # Explicitly hold on to the process handle. 
        # This works around an issue whereby Wait-Process would fail to find build_editor_proc 
        $build_editor_handle = $build_editor_proc.Handle

        Wait-Process -Id (Get-Process -InputObject $build_editor_proc).id
        if ($build_editor_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Win64 Development Editor. Error: $($build_editor_proc.ExitCode)"
            Throw "Failed to build Win64 Development Editor"
        }
    Finish-Event "build-editor" "build-unreal-gdk-example-project-:windows:"

    # Invoke the GDK commandlet to generate schema and snapshot. Note: this needs to be run prior to cooking 
    Start-Event "generate-schema" "build-unreal-gdk-example-project-:windows:"
        pushd "UnrealEngine/Engine/Binaries/Win64"
            Start-Process -Wait -PassThru -NoNewWindow -FilePath ((Convert-Path .) + "\UE4Editor.exe") -ArgumentList @(`
                "$($game_home)/Game/GDKShooter.uproject", `
                "-run=GenerateSchemaAndSnapshots", `
                "-MapPaths=`"/Maps/FPS-Start_Small`""
            )

            $core_gdk_schema_path = "$($gdk_home)\SpatialGDK\Extras\schema\*"
            $schema_std_lib_path = "$($gdk_home)\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\schema\*"
            New-Item -Path "$($game_home)\spatial\schema\unreal" -Name "gdk" -ItemType Directory -Force
            New-Item -Path "$($game_home)\spatial" -Name "\build\dependencies\schema\standard_library" -ItemType Directory -Force
            Copy-Item "$($core_gdk_schema_path)" -Destination "$($game_home)\spatial\schema\unreal\gdk" -Force -Recurse
            Copy-Item "$($schema_std_lib_path)" -Destination "$($game_home)\spatial\build\dependencies\schema\standard_library" -Force -Recurse
        popd
    Finish-Event "generate-schema" "build-unreal-gdk-example-project-:windows:"

    Start-Event "build-win64-client" "build-unreal-gdk-example-project-:windows:"
        $build_client_proc = Start-Process -PassThru -NoNewWindow -FilePath $build_script_path -ArgumentList @(`
            "GDKShooter", `
            "Win64", `
            "Development", `
            "GDKShooter.uproject"
        )       
        $build_client_handle = $build_client_proc.Handle
        Wait-Process -Id (Get-Process -InputObject $build_client_proc).id
        if ($build_client_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Win64 Development Client. Error: $($build_client_proc.ExitCode)"
            Throw "Failed to build Win64 Development Client"
        }
    Finish-Event "build-win64-client" "build-unreal-gdk-example-project-:windows:"

    Start-Event "build-linux-worker" "build-unreal-gdk-example-project-:windows:"
        $build_server_proc = Start-Process -PassThru -NoNewWindow -FilePath $build_script_path -ArgumentList @(`
            "GDKShooterServer", `
            "Linux", `
            "Development", `
            "GDKShooter.uproject"
        )       
        $build_server_handle = $build_server_proc.Handle
        Wait-Process -Id (Get-Process -InputObject $build_server_proc).id

        if ($build_server_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Linux Development Server. Error: $($build_server_proc.ExitCode)"
            Throw "Failed to build Linux Development Server"
        }
    Finish-Event "build-linux-worker" "build-unreal-gdk-example-project-:windows:"

    # Deploy the project to SpatialOS
    &$PSScriptRoot"\deploy.ps1"
popd