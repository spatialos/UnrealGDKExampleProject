param(
  [string] $launch_deployment = "false",
  [string] $gdk_branch_name
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

            # Send a Slack notification with a link to the new deployment and to the build.
            # Read Slack webhook secret from the vault and extract the Slack webhook URL from it.
            $slack_webhook_secret = "$(imp-ci secrets read --environment=production --buildkite-org=improbable --secret-type=slack-webhook --secret-name=unreal-gdk-slack-web-hook)"
            $slack_webhook_url = $slack_webhook_secret | ConvertFrom-Json | %{$_.url}

            $deployment_url = "https://console.improbable.io/projects/${project_name}/deployments/${deployment_name}/overview"
            $build_url = "$env:BUILDKITE_BUILD_URL"

            $json_message = [ordered]@{
                text = "Example Project build succeeded and a deployment has been launched."
                attachments= @(
                        @{
                            fallback = "Find deployment here: $deployment_url and build here: $build_url"
                            fields= @(
                                    @{
                                        title = "GDK branch"
                                        value = "$gdk_branch_name"
                                        short = "true"
                                    }
                                    @{
                                        title = "Example Project branch"
                                        value = "$env:BUILDKITE_BRANCH"
                                        short = "true"
                                    }
                                )
                            actions= @(
                                    @{
                                        type = "button"
                                        text = "take me to deployment"
                                        url = "$deployment_url"
                                        style = "primary"
                                    }
                                    @{
                                        type = "button"
                                        text = "take me to build"
                                        url = "$build_url"
                                        style = "primary"
                                    }
                                )
                        }
                    )
                }

            $json_request = $json_message | ConvertTo-Json -Depth 10

            Invoke-WebRequest -UseBasicParsing "$slack_webhook_url" -ContentType "application/json" -Method POST -Body "$json_request"

        }
        else {
            Write-Log 'By default, deployment will not be launched. To launch a deployment, pass in the following environment variable when starting a build from BuildKite: START_DEPLOYMENT="true"'
        }

    Finish-Event "launch-deployment" "deploy-unreal-gdk-example-project-:windows:"

popd
Finish-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
