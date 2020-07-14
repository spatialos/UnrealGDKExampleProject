param(
  [string] $exampleproject_home = (get-item "$($PSScriptRoot)").parent.FullName, ## The root of the repo
  [string] $project_name = "unreal_gdk",
)

. "$PSScriptRoot\common.ps1"

pushd "$exampleproject_home"
    pushd "spatial"
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
    popd
popd
