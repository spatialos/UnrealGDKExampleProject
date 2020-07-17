param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $deployment_launch_configuration = "one_worker_test.json",
  [string] $deployment_snapshot_path = "snapshots/Control_small.snapshot",
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
$engine_commit_formated_hash = Get-Env-Variable-Value-Or-Default -environment_variable_name "ENGINE_COMMIT_FORMATED_HASH" -default_value "0"
$android_autotest = Get-Meta-Data -variable_name "android-autotest"           

$gdk_home = "$exampleproject_home\Game\Plugins\UnrealGDK"
$parent_event_name = "build-unreal-gdk-example-project-:windows:"
$game_project = "$exampleproject_home/Game/GDKShooter.uproject"


pushd "$exampleproject_home"
    Start-Event "clone-gdk-plugin" $parent_event_name
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
    Finish-Event "clone-gdk-plugin" $parent_event_name

    Start-Event "get-gdk-head-commit" $parent_event_name
        pushd $gdk_home
            # Get the short commit hash of this gdk build for later use in assembly name
            $gdk_commit_hash = (git rev-parse HEAD).Substring(0,6)
            Write-Output "GDK at commit: $gdk_commit_hash on branch $gdk_branch_name"
        popd
    Finish-Event "get-gdk-head-commit" $parent_event_name

    Start-Event "set-up-gdk-plugin" $parent_event_name
        pushd $gdk_home
            # Invoke the GDK's setup script
            &"$($gdk_home)\ci\setup-gdk.ps1"
        popd
    Finish-Event "set-up-gdk-plugin" $parent_event_name

    # Use the cached engine version or set it up if it has not been cached yet.
    Start-Event "set-up-engine" $parent_event_name

        &"$($gdk_home)\ci\get-engine.ps1" -unreal_path "$unreal_engine_symlink_dir"

    Finish-Event "set-up-engine" $parent_event_name

    Start-Event "associate-uproject-with-engine" $parent_event_name
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
    Finish-Event "associate-uproject-with-engine" $parent_event_name

    $build_script_path = "$($gdk_home)\SpatialGDK\Build\Scripts\BuildWorker.bat"

    Start-Event "build-editor" $parent_event_name
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
    Finish-Event "build-editor" $parent_event_name

    if($android_autotest -ne 0){
        #generate-auth-ken need newest spatial
        Start-Event "spatial-update" $parent_event_name
            $build_configs_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
                "update"
            )

            if ($build_configs_process.ExitCode -ne 0) {
                Write-Log "Failed to update spatial. Error: $($build_configs_process.ExitCode)"
                Throw "Failed to update spatial"
            }
        Finish-Event "spatial-update" $parent_event_name
    }

    # Invoke the GDK commandlet to generate schema and snapshot. Note: this needs to be run prior to cooking 
    Start-Event "generate-schema" $parent_event_name
        $win64_folder = "${unreal_engine_symlink_dir}/Engine/Binaries/Win64"
        pushd $win64_folder
            $UE4Editor=((Convert-Path .) + "\UE4Editor-Cmd.exe")
            $schema_gen_proc = Start-Process -PassThru -NoNewWindow -FilePath $UE4Editor -ArgumentList @(`
                "$game_project", `
                "-run=CookAndGenerateSchema", `
                "-targetplatform=LinuxServer", `
                "-SkipShaderCompile", `
                "-map=`"/Maps/Control_small`""
            )
            $schema_gen_handle = $schema_gen_proc.Handle
            Wait-Process -InputObject $schema_gen_proc
            if ($schema_gen_proc.ExitCode -ne 0) {
                Write-Log "Failed to generate schema. Error: $($schema_gen_proc.ExitCode)"
                Throw "Failed to generate schema"
            }
            
            $snapshot_gen_proc = Start-Process -PassThru -NoNewWindow -FilePath $UE4Editor -ArgumentList @(`
                "$game_project", `
                "-run=GenerateSnapshot", `
                "-MapPaths=`"/Maps/Control_small`""
            )
            $snapshot_gen_handle = $snapshot_gen_proc.Handle
            Wait-Process -InputObject $snapshot_gen_proc
            if ($snapshot_gen_proc.ExitCode -ne 0) {
                Write-Log "Failed to generate snapshot. Error: $($snapshot_gen_proc.ExitCode)"
                Throw "Failed to generate snapshot"
            }
        popd
    Finish-Event "generate-schema" $parent_event_name

    if($android_autotest -eq 0){
        Start-Event "build-win64-client" $parent_event_name
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
        Finish-Event "build-win64-client" $parent_event_name
    }

    Start-Event "build-linux-worker" $parent_event_name
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
    Finish-Event "build-linux-worker" $parent_event_name

    if($android_autotest -ne 0){
        Start-Event "change-runtime-settings" $parent_event_name
            $proc = Start-Process -PassThru -NoNewWindow -FilePath "python" -ArgumentList @(`
                "ci/change-runtime-settings.py", `
                "$exampleproject_home"
            )
            Wait-Process -InputObject $proc

            $DefaultEngine = "$exampleproject_home\Game\Config\DefaultEngine.ini"
            $DefaultEngineContent = Get-Content -Path $DefaultEngine
            Write-Host $DefaultEngineContent    
        Finish-Event "change-runtime-settings" $parent_event_name
    }
    
    # Deploy the project to SpatialOS
    &$PSScriptRoot"\deploy.ps1" -launch_deployment "$launch_deployment" -gdk_branch_name "$gdk_branch_name" -parent_event_name "$parent_event_name"
    
    Start-Event "build-android-client" $parent_event_name          
        $auth_token = Get-Meta-Data -variable_name "auth-token"
        $deployment_name = Get-Meta-Data -variable_name "deployment-name-$($env:STEP_NUMBER)"
        Write-Output "auth_token: $auth_token"
        Write-Output "deployment_name: $deployment_name"
        $cookflavor = "Multi"
        Set-Meta-Data -variable_name "android-flavor" -variable_value $cookflavor
        if($android_autotest -ne 0){
            $cmdline="connect.to.spatialos -workerType UnrealClient -OverrideSpatialNetworking +devauthToken $auth_token +deployment $deployment_name +linkProtocol Tcp"
        }
        else{
            $cmdline=""
        }
        $argumentlist = @(`
            "-ScriptsForProject=$game_project", `
            "BuildCookRun", `
            "-nocompileeditor", `
            "-nop4", `
            "-project=$game_project", `
            "-cook", `
            "-stage", `
            "-archive", `
            "-archivedirectory=$($exampleproject_home)/cooked-android-$engine_commit_formated_hash", `
            "-package", `
            "-clientconfig=Development", `
            "-ue4exe=$($unreal_engine_symlink_dir)/Engine/Binaries/Win64/UE4Editor-Cmd.exe", `
            "-pak", `
            "-prereqs", `
            "-nodebuginfo", `
            "-targetplatform=Android", `
            "-cookflavor=$cookflavor", `
            "-build", `
            "-utf8output", `
            "-compile", `
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
        Set-Meta-Data -variable_name "$engine_commit_formated_hash-build-android-job-id" -variable_value "$env:BUILDKITE_JOB_ID"
        Set-Meta-Data -variable_name "$engine_commit_formated_hash-build-android-queue-id" -variable_value "$env:BUILDKITE_AGENT_META_DATA_QUEUE"
    Finish-Event "build-android-client" $parent_event_name
popd
