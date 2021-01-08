param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName ## The root of the repo  
)

. "$PSScriptRoot\common.ps1"

$gdk_repo = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_REPOSITORY" -default_value "git@github.com:spatialos/UnrealGDK.git"
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$project_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_PROJECT_NAME" -default_value "unreal_gdk"

Start-Event "get-gdk-head-commit" "generate-auth-token-and-deployment-:windows:" 
    # Get the short commit hash of this gdk build for later use in assembly name
    $gdk_commit_hash = (git ls-remote --head $gdk_repo $gdk_branch_name).Substring(0,6)
    
    # This is used during the slack-notify step
    buildkite-agent meta-data set "gdk_commit_hash" "$gdk_commit_hash"
    Write-Output "GDK at commit: $gdk_commit_hash on branch $gdk_branch_name"
Finish-Event "get-gdk-head-commit" "generate-auth-token-and-deployment-:windows:"

Start-Event "generate-project-name" "generate-auth-token-and-deployment-:windows:"
    $date_and_time = Get-Date -Format "MMdd_HHmm"
    $engine_version_count = buildkite-agent meta-data get "engine-version-count"
    for ($i = 0; $i -lt $engine_version_count; $i++){
        $index_string = "$($i+1)"
        $random_salt = -join ((48..57) + (97..122) | Get-Random -Count 4 | ForEach-Object {[char]$_})
        $deployment_name = "epci$(${index_string})_${random_salt}_${date_and_time}_$($gdk_commit_hash)"
        Write-Output "deployment_name-$index_string=$deployment_name"
        buildkite-agent meta-data set "deployment-name-$index_string" "$deployment_name"
    }
Finish-Event "generate-project-name" "generate-auth-token-and-deployment-:windows:"

pushd "$exampleproject_home" 
    pushd "spatial"
        Start-Event "generate-auth-token" "generate-auth-token-and-deployment-:windows:"
            $dev_auth_token_result = spatial project auth dev-auth-token create --description="Token generated for Example Project CI" --lifetime=24h --project_name=$project_name | Out-String
            $found_dev_token = $dev_auth_token_result -match 'token_secret:\\"(.+)\\"'
            if ($found_dev_token -eq 0) {
                Write-Log "Failed to find dev auth token"
                Throw "Unable to create a development authentication token. Please take a look at the logs."
            }

            $auth_token = $matches[1]
            Write-Output "auth_token=$auth_token"
            buildkite-agent meta-data set "auth-token" "$auth_token"
        Finish-Event "generate-auth-token" "generate-auth-token-and-deployment-:windows:"
    popd
popd
