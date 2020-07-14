param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName ## The root of the repo  
)

. "$PSScriptRoot\common.ps1"

$gdk_repo = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_REPOSITORY" -default_value ""
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$project_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_PROJECT_NAME" -default_value "unreal_gdk"

$gdk_home = "${exampleproject_home}\Game\Plugins\UnrealGDK"
Start-Event "clone-gdk-plugin" "build-unreal-gdk-example-project-:windows:"
    Write-Output "gdk_repo:$gdk_repo"
    Write-Output "gdk_branch_name:$gdk_branch_name"
    Write-Output "project_name:$project_name"

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
        Write-Output "GDK at commit: $gdk_commit_hash on branch $gdk_branch_name"
    popd
Finish-Event "get-gdk-head-commit" "build-unreal-gdk-example-project-:windows:"

Start-Event "generate-project-name" "build-unreal-gdk-example-project-:windows:"
    $date_and_time = Get-Date -Format "MMdd_HHmm"        
    $engine_version_count = Get-Meta-Data -variable_name "engine-version-count" -default_value "1"
    for ($i = 0; $i -lt $engine_version_count; $i++){
        $index = "$($i+1)"
        $deployment_name = "exampleproject$(${index})_${date_and_time}_$($gdk_commit_hash)"
        Set-Meta-Data -variable_name "deployment-name-$index" "$deployment_name"
    }
Finish-Event "generate-project-name" "build-unreal-gdk-example-project-:windows:"

pushd "$exampleproject_home" 
    pushd "spatial"    
        # Start-Event "prepare-for-run" "deploy-unreal-gdk-example-project-:windows:"
        #     $prepare_for_run_process = Start-Process -Wait -PassThru -NoNewWindow -FilePath "spatial" -ArgumentList @(`
        #         "prepare-for-run", `
        #         "--log_level=debug"
        #     )

        #     if ($prepare_for_run_process.ExitCode -ne 0) {
        #         Write-Output "Failed to prepare for a Spatial cloud launch. Error: $($prepare_for_run_process.ExitCode)"
        #         Throw "Spatial prepare for run failed"
        #     }
        # Finish-Event "prepare-for-run" "deploy-unreal-gdk-example-project-:windows:"

        Start-Event "generate-auth-token" "deploy-unreal-gdk-example-project-:windows:"
            Write-Output "spatial update"
            spatial update

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
            Write-Output "auth_token:$auth_token"
        Finish-Event "generate-auth-token" "deploy-unreal-gdk-example-project-:windows:"    
    popd
popd
