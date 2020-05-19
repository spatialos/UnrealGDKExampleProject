param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $gdk_repo = "git@github.com:spatialos/UnrealGDK.git",
  [string] $gcs_publish_bucket = "io-internal-infra-unreal-artifacts-production/UnrealEngine",
  [string] $deployment_launch_configuration = "one_worker_test.json",
  [string] $deployment_snapshot_path = "snapshots/FPS-Start_Small.snapshot",
  [string] $deployment_cluster_region = "eu",
  [string] $project_name = "unreal_gdk",
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build. Should ultimately resolve to "C:\b\<number>\".
  [string] $unreal_engine_symlink_dir = "$build_home\UnrealEngine"
)

. "$PSScriptRoot\common.ps1"

# When a build is launched custom environment variables can be specified.
# Parse them here to use the set value or the default.
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$launch_deployment = Get-Env-Variable-Value-Or-Default -environment_variable_name "START_DEPLOYMENT" -default_value "true"

$gdk_home = "${exampleproject_home}\Game\Plugins\UnrealGDK"

pushd "$exampleproject_home"
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
            $gdk_commit_hash = (git rev-parse HEAD).Substring(0,6)
            Write-Log "GDK at commit: $gdk_commit_hash on branch $gdk_branch_name"
        popd
    Finish-Event "get-gdk-head-commit" "build-unreal-gdk-example-project-:windows:"

    Start-Event "set-up-gdk-plugin" "build-unreal-gdk-example-project-:windows:"
        pushd $gdk_home
            # Invoke the GDK's setup script
            &"$($gdk_home)\ci\setup-gdk.ps1"
        popd
    Finish-Event "set-up-gdk-plugin" "build-unreal-gdk-example-project-:windows:"

    # Use the cached engine version or set it up if it has not been cached yet.
    Start-Event "set-up-engine" "build-unreal-gdk-example-project-:windows:"

        &"$($gdk_home)\ci\get-engine.ps1" -unreal_path "$unreal_engine_symlink_dir"

    Finish-Event "set-up-engine" "build-unreal-gdk-example-project-:windows:"

    Start-Event "associate-uproject-with-engine" "build-unreal-gdk-example-project-:windows:"
        pushd $unreal_engine_symlink_dir
            $unreal_version_selector_path = "Engine\Binaries\Win64\UnrealVersionSelector.exe"

            $find_engine_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath $unreal_version_selector_path -ArgumentList @(`
                "-switchversionsilent", `
                "${exampleproject_home}\Game\GDKShooter.uproject", `
                "$unreal_engine_symlink_dir"
            )

            if ($find_engine_process.ExitCode -ne 0) {
                Write-Log "Failed to set Unreal Engine association for the project. Error: $($find_engine_process.ExitCode)"
                Throw "Failed to set Engine association"
            }
        popd
    Finish-Event "associate-uproject-with-engine" "build-unreal-gdk-example-project-:windows:"


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

        Wait-Process -InputObject $build_editor_proc
        if ($build_editor_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Win64 Development Editor. Error: $($build_editor_proc.ExitCode)"
            Throw "Failed to build Win64 Development Editor"
        }
    Finish-Event "build-editor" "build-unreal-gdk-example-project-:windows:"

    # Invoke the GDK commandlet to generate schema and snapshot. Note: this needs to be run prior to cooking 
    Start-Event "generate-schema" "build-unreal-gdk-example-project-:windows:"
        pushd "${unreal_engine_symlink_dir}/Engine/Binaries/Win64"
            $schema_gen_proc = Start-Process -PassThru -NoNewWindow -FilePath ((Convert-Path .) + "\UE4Editor.exe") -ArgumentList @(`
                "$($exampleproject_home)/Game/GDKShooter.uproject", `
                "-run=CookAndGenerateSchema", `
                "-targetplatform=LinuxServer", `
                "-SkipShaderCompile", `
                "-map=`"/Maps/FPS-Start_Small`""
            )
            $schema_gen_handle = $schema_gen_proc.Handle
            Wait-Process -InputObject $schema_gen_proc
            if ($schema_gen_proc.ExitCode -ne 0) {
                Write-Log "Failed to generate schema. Error: $($schema_gen_proc.ExitCode)"
                Throw "Failed to generate schema"
            }
            
            $snapshot_gen_proc = Start-Process -PassThru -NoNewWindow -FilePath ((Convert-Path .) + "\UE4Editor.exe") -ArgumentList @(`
                "$($exampleproject_home)/Game/GDKShooter.uproject", `
                "-run=GenerateSnapshot", `
                "-MapPaths=`"/Maps/FPS-Start_Small`""
            )
            $snapshot_gen_handle = $snapshot_gen_proc.Handle
            Wait-Process -InputObject $snapshot_gen_proc
            if ($snapshot_gen_proc.ExitCode -ne 0) {
                Write-Log "Failed to generate snapshot. Error: $($snapshot_gen_proc.ExitCode)"
                Throw "Failed to generate snapshot"
            }
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
        Wait-Process -InputObject $build_client_proc
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
        Wait-Process -InputObject $build_server_proc

        if ($build_server_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Linux Development Server. Error: $($build_server_proc.ExitCode)"
            Throw "Failed to build Linux Development Server"
        }
    Finish-Event "build-linux-worker" "build-unreal-gdk-example-project-:windows:"

    Start-Event "build-android-client" "build-unreal-gdk-example-project-:windows:"
        $unreal_uat_path = "${unreal_engine_symlink_dir}\Engine\Build\BatchFiles\RunUAT.bat"
        $build_server_proc = Start-Process -PassThru -NoNewWindow -FilePath $unreal_uat_path -ArgumentList @(`
            "-ScriptsForProject=$($exampleproject_home)/Game/GDKShooter.uproject", `
            "BuildCookRun", `
            "-nocompileeditor", `
            "-nop4", `
            "-project=$($exampleproject_home)/Game/GDKShooter.uproject", `
            "-cook", `
            "-stage", `
            "-archive", `
            "-archivedirectory=$($exampleproject_home)/cooked-android", `
            "-package", `
            "-clientconfig=Development", `
            "-ue4exe=$($unreal_engine_symlink_dir)/Engine/Binaries/Win64/UE4Editor-Cmd.exe", `
            "-pak", `
            "-prereqs", `
            "-nodebuginfo", `
            "-targetplatform=Android", `
            "-cookflavor=Multi", `
            "-build", `
            "-utf8output", `
            "-compile"
        )

        $build_server_handle = $build_server_proc.Handle
        Wait-Process -InputObject $build_server_proc

        if ($build_server_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Android Development Client. Error: $($build_server_proc.ExitCode)"
            Throw "Failed to build Android Development Client"
        }
    Finish-Event "build-android-client" "build-unreal-gdk-example-project-:windows:"

    # Deploy the project to SpatialOS
    &$PSScriptRoot"\deploy.ps1" -launch_deployment "$launch_deployment" -gdk_branch_name "$gdk_branch_name"
popd