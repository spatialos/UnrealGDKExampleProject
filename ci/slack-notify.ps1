. "$PSScriptRoot\common.ps1"

$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$launch_deployment = Get-Env-Variable-Value-Or-Default -environment_variable_name "START_DEPLOYMENT" -default_value "true"
$slack_channel = Get-Env-Variable-Value-Or-Default -environment_variable_name "SLACK_CHANNEL" -default_value "#unreal-gdk-builds"
$engine_version_count = buildkite-agent meta-data get "engine-version-count"
$project_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_PROJECT_NAME" -default_value "unreal_gdk"
$mac_build = Get-Env-Variable-Value-Or-Default -environment_variable_name "MAC_BUILD" -default_value "false"
$firebase_test = Get-Env-Variable-Value-Or-Default -environment_variable_name "FIREBASE_TEST" -default_value "false"
$gdk_commit_hash = buildkite-agent meta-data get "gdk_commit_hash"

# Send a Slack notification with a link to the new deployment and to the build.
Start-Event "slack-notify" "slack-notify"
    # Build Slack text
    if($env:NIGHTLY_BUILD -eq "true"){
        $slack_text = ":night_with_stars: Nightly build of *Example Project* succeeded."
    }
    elseif ($firebase_test -eq "true"){
        $slack_text = ":night_with_stars: Firebase Connection Tests for the *Example Project* succeeded."
    }else{
        $slack_text = "*Example Project* build by $env:BUILDKITE_BUILD_CREATOR succeeded."
    }

    # Read Slack webhook secret from the vault and extract the Slack webhook URL from it.
    $slack_webhook_secret = "$(imp-ci secrets read --environment=production --buildkite-org=improbable --secret-type=slack-webhook --secret-name=unreal-gdk-slack-web-hook)"
    $slack_webhook_url = $slack_webhook_secret | ConvertFrom-Json | %{$_.url}

    $gdk_commit_url = "https://github.com/spatialos/UnrealGDK/commit/${gdk_commit_hash}"
    $project_commit_url = "https://github.com/spatialos/UnrealGDKExampleProject/commit/$env:BUILDKITE_COMMIT"
    $build_url = "$env:BUILDKITE_BUILD_URL"
    $json_message = [ordered]@{
        text = "$slack_text"
        channel = "$slack_channel"
        attachments= @(
                @{
                    fallback = "Find build here: $build_url."
                    color = "good"
                    fields = @(
                            if ($firebase_test -eq "true") {
                                @{
                                    title = "Android Test Result"
                                    value = "succeeded"
                                    short = "true"
                                }
                                if($mac_build -eq "true") {
                                    @{
                                        title = "iOS Test Result"
                                        value = "succeeded"
                                        short = "true"
                                    }
                                }
                            }
                            @{
                                title = "Build Message"
                                value = "$env:BUILDKITE_MESSAGE".Substring(0, [System.Math]::Min(64, "$env:BUILDKITE_MESSAGE".Length)) 
                                short = "true"
                            }
                            @{
                                title = "Example Project branch"
                                value = "$env:BUILDKITE_BRANCH"
                                short = "true"
                            }
                            @{
                                title = "GDK branch"
                                value = "$gdk_branch_name"
                                short = "true"
                            }
                        )
                    actions = @(
                            @{
                                type = "button"
                                text = ":github: Project commit"
                                url = "$project_commit_url"
                                style = "primary"
                            }
                            @{
                                type = "button"
                                text = ":github: GDK commit"
                                url = "$gdk_commit_url"
                                style = "primary"
                            }
                            @{
                                type = "button"
                                text = ":buildkite: BK build"
                                url = "$build_url"
                                style = "primary"
                            }
                        )
                }
            )
        }

    if ($launch_deployment -eq "true") {
        for ($i = 0; $i -lt $engine_version_count; $i++){
            $deployment_name = buildkite-agent meta-data get "deployment-name-$($i+1)"
            $deployment_url = "https://console.improbable.io/projects/${project_name}/deployments/${deployment_name}/overview"
            $deployment_button = @{
                                type = "button"
                                text = ":cloud: Deployment $($i+1)"
                                url = "$deployment_url"
                                style = "primary"
                            }
            $json_message["attachments"][0]["actions"] += ($deployment_button)
        }
    }

    $json_request = $json_message | ConvertTo-Json -Depth 10

    Invoke-WebRequest -UseBasicParsing "$slack_webhook_url" -ContentType "application/json" -Method POST -Body "$json_request"
Finish-Event "slack-notify" "slack-notify"
