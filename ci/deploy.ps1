param(
  [string] $launch_deployment = "false",
  [string] $gdk_branch_name = "master"
)

. "$PSScriptRoot\common.ps1"


# Grab Artifacts
Start-Event "fetch-artifacts" "build-unreal-gdk-example-project-:windows:"
    $schema_path = "Game\Content\Spatial"
    New-Item -ItemType directory -Path staging | Out-Null
    if (-Not (Test-Path $schema_path)) {
        New-Item -ItemType directory -Path $schema_path
    }
    buildkite-agent artifact download "*Schema.zip" staging
    7z x staging\artifacts\Schema.zip -aoa -ospatial
    buildkite-agent artifact download "*Snapshots.zip" staging
    7z x staging\artifacts\Snapshots.zip -aoa -ospatial
Finish-Event "fetch-artifacts" "build-unreal-gdk-example-project-:windows:"

Start-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
    # deployment_name is created during the generate-auth_token-and-deployment-name step
    $deployment_name = buildkite-agent meta-data get "deployment-name-$($env:STEP_NUMBER)"    
    $assembly_name = "$($deployment_name)_asm"
    $runtime_version = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_RUNTIME_VERSION" -default_value ""
    $project_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_PROJECT_NAME" -default_value "unreal_gdk"
    
pushd "spatial"
    Start-Event "build-worker-configurations" "deploy-unreal-gdk-example-project-:windows:"
        $build_configs_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "build", `
            "build-config"
        )

        if ($build_configs_process.ExitCode -ne 0) {
            Write-Log "Failed to build worker configurations for the project. Error: $($build_configs_process.ExitCode)"
            Throw "Failed to build worker configurations"
        }
    Finish-Event "build-worker-configurations" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "prepare-for-run" "deploy-unreal-gdk-example-project-:windows:"
        $prepare_for_run_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "prepare-for-run", `
            "--log_level=debug"
        )

        if ($prepare_for_run_process.ExitCode -ne 0) {
            Write-Log "Failed to prepare for a Spatial cloud launch. Error: $($prepare_for_run_process.ExitCode)"
            Throw "Spatial prepare for run failed"
        }
    Finish-Event "prepare-for-run" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "upload-assemblies" "deploy-unreal-gdk-example-project-:windows:"
        $upload_assemblies_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "cloud", `
            "upload", `
            "$assembly_name", `
            "--project_name=$project_name", `
            "--log_level=debug", `
            "--force"
        )

        if ($upload_assemblies_process.ExitCode -ne 0) {
            Write-Log "Failed to upload assemblies to cloud. Error: $($upload_assemblies_process.ExitCode)"
            Throw "Failed to upload assemblies"
        }
    Finish-Event "upload-assemblies" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "launch-deployment" "deploy-unreal-gdk-example-project-:windows:"
        # Determine whether deployment should be launched (it will by default)
        if ($launch_deployment -eq "true") {
            $launch_deployment_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
                "cloud", `
                "launch", `
                "$assembly_name", `
                "$deployment_launch_configuration", `
                "$deployment_name", `
                "--runtime_version=$runtime_version", `
                "--project_name=$project_name", `
                "--snapshot=snapshots/$main_map_name.snapshot", `
                "--cluster_region=$deployment_cluster_region", `
                "--log_level=debug", `
                "--tags=ttl_1_hours,dev_login", `
                "--deployment_description=`"Engine commit: $($env:ENGINE_COMMIT_HASH)`"" `
            )

            if ($launch_deployment_process.ExitCode -ne 0) {
                Write-Log "Failed to launch a Spatial cloud deployment. Error: $($launch_deployment_process.ExitCode)"
                Throw "Deployment launch failed"
            }

        } else {
            Write-Log "Deployment will not be launched as you have passed in an argument specifying that it should not be (START_DEPLOYMENT=${launch_deployment}). Remove it to have your build launch a deployment."
        }
    Finish-Event "launch-deployment" "deploy-unreal-gdk-example-project-:windows:"
popd
Finish-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
