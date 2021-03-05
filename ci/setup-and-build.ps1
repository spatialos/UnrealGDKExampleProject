param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $deployment_launch_configuration = "one_worker_test.json",
  [string] $deployment_cluster_region = "eu",
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build. Should ultimately resolve to "C:\b\<number>\".
  [string] $unreal_engine_symlink_dir = "$build_home\UnrealEngine"
)

. "$PSScriptRoot\common.ps1"

# When a build is launched custom environment variables can be specified.
# Parse them here to use the set value or the default.
$gdk_repo = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_REPOSITORY" -default_value ""
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$launch_deployment = Get-Env-Variable-Value-Or-Default -environment_variable_name "START_DEPLOYMENT" -default_value "true"
$engine_commit_formatted_hash = Get-Env-Variable-Value-Or-Default -environment_variable_name "ENGINE_COMMIT_FORMATTED_HASH" -default_value "0"
$main_map_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "MAIN_MAP_NAME" -default_value "Control_Small"
$run_firebase_test = Get-Env-Variable-Value-Or-Default -environment_variable_name "FIREBASE_TEST" -default_value "false"

$gdk_home = "$exampleproject_home\Game\Plugins\UnrealGDK"
$game_project = "$exampleproject_home/Game/GDKShooter.uproject"

pushd "$exampleproject_home"

    New-Item -Path "$exampleproject_home\Game\Content\Spatial\" -ItemType Directory 
    buildkite-agent artifact download "*SchemaDatabase.uasset" "$exampleproject_home"

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
                "$game_project", `
                "$unreal_engine_symlink_dir"
            )

            if ($find_engine_process.ExitCode -ne 0) {
                Write-Log "Failed to set Unreal Engine association for the project. Error: $($find_engine_process.ExitCode)"
                Throw "Failed to set Engine association"
            }
        popd
    Finish-Event "associate-uproject-with-engine" "build-unreal-gdk-example-project-:windows:"

    $build_script_path = "$($gdk_home)\SpatialGDK\Build\Scripts\BuildWorker.bat"
    Start-Event "build-win64-client" "build-unreal-gdk-example-project-:windows:"
        $build_client_proc = Start-Process -PassThru -NoNewWindow -FilePath $build_script_path -ArgumentList @(`
            "GDKShooter", `
            "Win64", `
            "Development", `
            "GDKShooter.uproject", `
            "-nocompile"
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
            "GDKShooter.uproject", `
            "-nocompile"
        )
        $build_server_handle = $build_server_proc.Handle
        Wait-Process -InputObject $build_server_proc

        if ($build_server_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Linux Development Server. Error: $($build_server_proc.ExitCode)"
            Throw "Failed to build Linux Development Server"
        }
    Finish-Event "build-linux-worker" "build-unreal-gdk-example-project-:windows:"

    if ($run_firebase_test -eq "true") {
        # Prepare Android Project Settings for Firebase
        Start-Event "change-runtime-settings" "build-unreal-gdk-example-project-:windows:"
            $proc = Start-Process -PassThru -NoNewWindow -FilePath "py" -ArgumentList @(`
                "-3", `
                "ci/change-runtime-settings.py", `
                "`"$exampleproject_home`""
            )
            Wait-Process -InputObject $proc
        Finish-Event "change-runtime-settings" "build-unreal-gdk-example-project-:windows:"
    }
        
    Start-Event "build-android-client" "build-unreal-gdk-example-project-:windows:"          
        $auth_token = buildkite-agent meta-data get "auth-token"
        $deployment_name = buildkite-agent meta-data get "deployment-name-$($env:STEP_NUMBER)"
        Write-Host "Cloud deployment to connect to: $deployment_name"
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
            "-ue4exe=$($unreal_engine_symlink_dir)/Engine/Binaries/Win64/UE4Editor-Cmd.exe", `
            "-pak", `
            "-prereqs", `
            "-nodebuginfo", `
            "-targetplatform=Android", `
            "-cookflavor=ETC2", `
            "-build", `
            "-utf8output", `
            "-cmdline=`"${cmdline}`""
        )

        $unreal_uat_path = "${unreal_engine_symlink_dir}\Engine\Build\BatchFiles\RunUAT.bat"
        $build_server_proc = Start-Process -PassThru -NoNewWindow -FilePath $unreal_uat_path -ArgumentList $argumentlist

        $build_server_handle = $build_server_proc.Handle
        Wait-Process -InputObject $build_server_proc

        if ($build_server_proc.ExitCode -ne 0) {
            Write-Log "Failed to build Android Development Client. Error: $($build_server_proc.ExitCode)"
            Throw "Failed to build Android Development Client"
        }
        # Store the queue and job id to be able to construct the GCS path when running the firebase tests.
        buildkite-agent meta-data set "$engine_commit_formatted_hash-build-android-job-id" "$env:BUILDKITE_JOB_ID"
        buildkite-agent meta-data set "$engine_commit_formatted_hash-build-android-queue-id" "$env:BUILDKITE_AGENT_META_DATA_QUEUE"
    Finish-Event "build-android-client" "build-unreal-gdk-example-project-:windows:"

    # Deploy the project to SpatialOS
    &$PSScriptRoot"\deploy.ps1" -launch_deployment "$launch_deployment" -gdk_branch_name "$gdk_branch_name"
popd
