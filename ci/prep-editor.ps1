param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $build_home = (Get-Item "$($PSScriptRoot)").parent.parent.FullName, ## The root of the entire build. Should ultimately resolve to "C:\b\<number>\".
  [string] $unreal_engine_symlink_dir = "$build_home\UnrealEngine"
)

. "$PSScriptRoot\common.ps1"

# When a build is launched custom environment variables can be specified.
# Parse them here to use the set value or the default.
$gdk_repo = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_REPOSITORY" -default_value ""
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$engine_commit_formatted_hash = Get-Env-Variable-Value-Or-Default -environment_variable_name "ENGINE_COMMIT_FORMATTED_HASH" -default_value "0"
$main_map_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "MAIN_MAP_NAME" -default_value "Control_Small"
$run_firebase_test = Get-Env-Variable-Value-Or-Default -environment_variable_name "FIREBASE_TEST" -default_value "false"

$gdk_home = "$exampleproject_home\Game\Plugins\UnrealGDK"
$game_project = "$exampleproject_home\Game\GDKShooter.uproject"

pushd "$exampleproject_home"
    Start-Event "clone-gdk-plugin" "prep-editor"
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
    Finish-Event "clone-gdk-plugin" "prep-editor"

    Start-Event "set-up-gdk-plugin" "prep-editor"
        pushd $gdk_home
            # Invoke the GDK's setup script
            &"$($gdk_home)\ci\setup-gdk.ps1"
        popd
    Finish-Event "set-up-gdk-plugin" "prep-editor"

    # Use the cached engine version or set it up if it has not been cached yet.
    Start-Event "set-up-engine" "prep-editor"
        &"$($gdk_home)\ci\get-engine.ps1" -unreal_path "$unreal_engine_symlink_dir"
    Finish-Event "set-up-engine" "prep-editor"

    Start-Event "associate-uproject-with-engine" "prep-editor"
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
    Finish-Event "associate-uproject-with-engine" "prep-editor"

    $build_script_path = "$($gdk_home)\SpatialGDK\Build\Scripts\BuildWorker.bat"

    Start-Event "build-editor" "prep-editor"
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
    Finish-Event "build-editor" "prep-editor"

    # Invoke the GDK commandlet to generate schema and snapshot. Note: this needs to be run prior to cooking 
    Start-Event "generate-schema" "prep-editor"
        pushd "${unreal_engine_symlink_dir}\Engine\Binaries\Win64"
            $UE4Editor=((Convert-Path .) + "\UE4Editor-Cmd.exe")
            $schema_gen_proc = Start-Process -PassThru -NoNewWindow -FilePath $UE4Editor -ArgumentList @(`
                "$game_project", `
                "-run=CookAndGenerateSchema", `
                "-targetplatform=LinuxServer", `
                "-SkipShaderCompile", `
                "-map=`"/Maps/$main_map_name`""
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
                "-MapPaths=`"/Maps/$main_map_name`""
            )
            $snapshot_gen_handle = $snapshot_gen_proc.Handle
            Wait-Process -InputObject $snapshot_gen_proc
            if ($snapshot_gen_proc.ExitCode -ne 0) {
                Write-Log "Failed to generate snapshot. Error: $($snapshot_gen_proc.ExitCode)"
                Throw "Failed to generate snapshot"
            }
        popd
    Finish-Event "generate-schema" "prep-editor"

    # Zip the artifacts
    Start-Event "zip-artifacts" "prep-editor"
        Write-Log "Zipping Schema..."
        $zip_proc = Start-Process -Wait -PassThru -NoNewWindow "7z" -ArgumentList @(`
            "a", `
            "-mx3", `
            "artifacts\Schema.zip", `
            "$PSScriptRoot\..\spatial\schema" `
        )
        if ($zip_proc.ExitCode -ne 0) {
            Write-Log "Failed to zip schema. Error: $($zip_proc.ExitCode)"
            Throw "Schema Zip Failed"
        }

        Write-Log "Zipping Snapshots..."
        $zip_proc = Start-Process -Wait -PassThru -NoNewWindow "7z" -ArgumentList @(`
            "a", `
            "-mx3", `
            "artifacts\Snapshots.zip", `
            "$PSScriptRoot\..\spatial\snapshots" `
        )

        if ($zip_proc.ExitCode -ne 0) {
            Write-Log "Failed to zip snapshots. Error: $($zip_proc.ExitCode)"
            Throw "Snapshot Zip Failed"
        }
    Finish-Event "zip-artifacts" "prep-editor"
popd
