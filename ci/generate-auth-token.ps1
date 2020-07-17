param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName ## The root of the repo  
)

. "$PSScriptRoot\common.ps1"

$gdk_repo = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_REPOSITORY" -default_value ""
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$project_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_PROJECT_NAME" -default_value "unreal_gdk"
$parent_event_name = "generate-auth-token-and-deployment-:windows:"

$gdk_home = "${exampleproject_home}\Game\Plugins\UnrealGDK"
Start-Event "clone-gdk-plugin" $parent_event_name
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
Finish-Event "clone-gdk-plugin" $parent_event_name

Start-Event "get-gdk-head-commit" $parent_event_name
    pushd $gdk_home
        # Get the short commit hash of this gdk build for later use in assembly name
        $gdk_commit_hash = (git rev-parse HEAD).Substring(0,6)
        Write-Output "GDK at commit: $gdk_commit_hash on branch $gdk_branch_name"
    popd
Finish-Event "get-gdk-head-commit" $parent_event_name

Start-Event "generate-project-name" $parent_event_name
    $date_and_time = Get-Date -Format "MMdd_HHmm"        
    $engine_version_count = Get-Meta-Data -variable_name "engine-version-count"
    for ($i = 0; $i -lt $engine_version_count; $i++){
        $index = "$($i+1)"
        $deployment_name = "exampleproject$(${index})_${date_and_time}_$($gdk_commit_hash)"
        Set-Meta-Data -variable_name "deployment-name-$index" "$deployment_name"
    }
Finish-Event "generate-project-name" $parent_event_name

pushd "$exampleproject_home" 
    pushd "spatial"
        Start-Event "generate-auth-token" $parent_event_name
            $DESCRIPTION = "Unreal-GDK-Token" 
            $DEVAUTH_CREATE = spatial project auth dev-auth-token create --description=$DESCRIPTION --project_name=$project_name | Out-String
            Write-Output $DEVAUTH_CREATE
            $FOUND_DEV_TOKEN = $DEVAUTH_CREATE -match 'token_secret:\\"(.+)\\"'
            if ($FOUND_DEV_TOKEN -eq 0) {
                Write-Log "Failed to find dev auth token"
                Throw "dev auth token not found"
            }
            $auth_token = $matches[1]
            Set-Meta-Data -variable_name "auth-token" "$auth_token"
            Write-Output "auth_token:$auth_token"
        Finish-Event "generate-auth-token" $parent_event_name
    popd
popd
