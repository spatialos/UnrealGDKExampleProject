param(
  [string] $launch_deployment = "false",
  [string] $gdk_branch_name = "master"
)

. "$PSScriptRoot\common.ps1"

Start-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
    # Use the shortened commit hash gathered during GDK plugin clone and the current date and time to distinguish the deployment
    $date_and_time = Get-Date -Format "MMdd_HHmm"
    $deployment_name = "exampleproject$($env:STEP_NUMBER)_${date_and_time}_$($gdk_commit_hash)"
    $assembly_name = "$($deployment_name)_asm"

pushd "spatial"
    #generate-auth-ken need newest spatial
    Start-Event "spatial-update" "deploy-unreal-gdk-example-project-:windows:"
        $build_configs_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "update"
        )

        if ($build_configs_process.ExitCode -ne 0) {
            Write-Output "Failed to update spatial. Error: $($build_configs_process.ExitCode)"
            Throw "Failed to update spatial"
        }
    Finish-Event "spatial-update" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "build-worker-configurations" "deploy-unreal-gdk-example-project-:windows:"
        $build_configs_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "build", `
            "build-config"
        )

        if ($build_configs_process.ExitCode -ne 0) {
            Write-Output "Failed to build worker configurations for the project. Error: $($build_configs_process.ExitCode)"
            Throw "Failed to build worker configurations"
        }
    Finish-Event "build-worker-configurations" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "prepare-for-run" "deploy-unreal-gdk-example-project-:windows:"
        $prepare_for_run_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "prepare-for-run", `
            "--log_level=debug"
        )

        if ($prepare_for_run_process.ExitCode -ne 0) {
            Write-Output "Failed to prepare for a Spatial cloud launch. Error: $($prepare_for_run_process.ExitCode)"
            Throw "Spatial prepare for run failed"
        }
    Finish-Event "prepare-for-run" "deploy-unreal-gdk-example-project-:windows:"
    
    Start-Event "generate-auth-token" "deploy-unreal-gdk-example-project-:windows:"
        $DESCRIPTION = "Unreal-GDK-Token" 
        $DEVAUTH_CREATE = spatial project auth dev-auth-token create --description=$DESCRIPTION --project_name=$project_name | Out-String
        Write-Output $DEVAUTH_CREATE
        $FOUND_DEV_TOKEN = $DEVAUTH_CREATE -match 'token_secret:\\"(.+)\\"'
        if ($FOUND_DEV_TOKEN -eq 0) {
        	Write-Output "Failed to find dev auth token"
        	Throw "dev auth token not found"
        }
        $auth_token = $matches[1]
        Set-Meta-Data -variable_name "auth-token" "$auth_token"
    Finish-Event "generate-auth-token" "deploy-unreal-gdk-example-project-:windows:"

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
            Write-Output "Failed to upload assemblies to cloud. Error: $($upload_assemblies_process.ExitCode)"
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
                "--snapshot=$deployment_snapshot_path", `
                "--cluster_region=$deployment_cluster_region", `
                "--log_level=debug", `
                "--tags=ttl_1_hours", `
                "--deployment_description=`"Engine commit: $($env:ENGINE_COMMIT_HASH)`"" `
            )

            if ($launch_deployment_process.ExitCode -ne 0) {
                Write-Output "Failed to launch a Spatial cloud deployment. Error: $($launch_deployment_process.ExitCode)"
                Throw "Deployment launch failed"
            }

            Set-Meta-Data -variable_name "deployment-name-$($env:STEP_NUMBER)" -variable_value "$deployment_name"
            Set-Meta-Data -variable_name "project-name" -variable_value "$project_name"
            Set-Meta-Data -variable_name "gdk-commit-hash" -variable_value "$gdk_commit_hash"
        } else {
            Write-Output "Deployment will not be launched as you have passed in an argument specifying that it should not be (START_DEPLOYMENT=${launch_deployment}). Remove it to have your build launch a deployment."
        }
    Finish-Event "launch-deployment" "deploy-unreal-gdk-example-project-:windows:"

    Start-Event "add-dev-login-tag" "deploy-unreal-gdk-example-project-:windows:"
        $add_dev_login_tag = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
            "project", `
            "deployment", `
            "tags", `
            "add", `
            "$deployment_name", `
            "dev_login"
        )
        if ($add_dev_login_tag.ExitCode -ne 0) {
            Write-Output "Failed to add dev_login tag to the deployment. Error: $($add_dev_login_tag.ExitCode)"
            Throw "Failed to add dev_login"
        }
    Finish-Event "add-dev-login-tag" "deploy-unreal-gdk-example-project-:windows:"
popd
Finish-Event "deploy-game" "build-unreal-gdk-example-project-:windows:"
