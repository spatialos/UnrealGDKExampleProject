param(
  [string] $launch_deployment = "false"
)

. "$PSScriptRoot\common.ps1"

Start-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
    # Use the shortened commit hash gathered during GDK plugin clone and the current date and time to distinguish the deployment
    $date_and_time = Get-Date -Format "MMdd_HHmm"
    $deployment_name = "exampleproject_${date_and_time}_$($gdk_commit_hash)"
    $assembly_name = "$($deployment_name)_asm"

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

    Start-Event "uploading-assemblies" "deploy-unreal-gdk-example-project-:windows:"
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
    Finish-Event "uploading-assemblies" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "launching-deployment" "deploy-unreal-gdk-example-project-:windows:"
        # Determine whether deployment should be launched (by default it is not)
        If($launch_deployment -eq "true") {
            $launch_deployment_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
                "cloud", `
                "launch", `
                "$assembly_name", `
                "$deployment_launch_configuration", `
                "$deployment_name", `
                "--project_name=$project_name", `
                "--snapshot=$deployment_snapshot_path", `
                "--cluster_region=$deployment_cluster_region", `
                "--log_level=debug"
            )

            if ($launch_deployment_process.ExitCode -ne 0) {
                Write-Log "Failed to launch a Spatial cloud deployment. Error: $($launch_deployment_process.ExitCode)"
                Throw "Deployment launch failed"
            }
        }
        else {
            Write-Log 'By default, deployment will not be launched. To launch a deployment, pass in the following environment variable when starting a build from BuildKite: START_DEPLOYMENT="true"'
        }

    Finish-Event "launching-deployment" "deploy-unreal-gdk-example-project-:windows:"

popd
Finish-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
