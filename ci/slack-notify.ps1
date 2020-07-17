. "$PSScriptRoot\common.ps1"
$project_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "SPATIAL_PROJECT_NAME" -default_value "unreal_gdk"
$gdk_branch_name = Get-Env-Variable-Value-Or-Default -environment_variable_name "GDK_BRANCH" -default_value "master"
$launch_deployment = Get-Env-Variable-Value-Or-Default -environment_variable_name "START_DEPLOYMENT" -default_value "true"
$slack_channel = Get-Env-Variable-Value-Or-Default -environment_variable_name "SLACK_CHANNEL" -default_value "#mobile-buildkite"
$engine_version_count = Get-Meta-Data -variable_name "engine-version-count"
$gdk_commit_hash = Get-Meta-Data -variable_name "gdk_commit_hash"
$android_autotest = Get-Meta-Data -variable_name "android-autotest"
$ios_autotest = Get-Meta-Data -variable_name "ios-autotest"

# Send a Slack notification with a link to the new deployment and to the build.
Start-Event "slack-notify" "slack-notify"
    $build_result = "failed"
    if($android_succeed -eq $android_total -and $ios_succeed -eq $ios_total){
        $build_result = "succeeded"
    }
    # Build Slack text
    if ($env:NIGHTLY_BUILD -eq "1" -or $env:FIREBASE_AUTOTEST -eq "1") {
        $slack_text = ":night_with_stars: Nightly build of *Example Project* *$build_result*."
    } else {
        $slack_text = "Example Project build by ``$env:BUILDKITE_BUILD_CREATOR`` *$build_result*."
    }
    # Read Slack webhook secret from the vault and extract the Slack webhook URL from it.
    $slack_webhook_secret = "$(imp-ci secrets read --environment=production --buildkite-org=improbable --secret-type=slack-webhook --secret-name=unreal-gdk-slack-web-hook)"
    $slack_webhook_url = $slack_webhook_secret | ConvertFrom-Json | %{$_.url}

    Write-Output "slack_webhook_url:$slack_webhook_url"

    $gdk_commit_url = "https://github.com/spatialos/UnrealGDK/commit/${gdk_commit_hash}"
    $project_commit_url = "https://github.com/spatialos/UnrealGDKExampleProject/commit/$env:BUILDKITE_COMMIT"
    $build_url = "$env:BUILDKITE_BUILD_URL"
    $build_message = "$env:BUILDKITE_MESSAGE".Substring(0, [System.Math]::Min(64, "$env:BUILDKITE_MESSAGE".Length))     
    $buildkite_branch = $env:BUILDKITE_BRANCH

    $json_message = [ordered]@{
        text = "$slack_text"
        channel = "$slack_channel"
        attachments= @(
                @{
                    fallback = "Find build here: $build_url."
                    fields = @(
                            if ($android_autotest -eq "1") {                                
                                $android_succeed = Get-Meta-Data -variable_name 'firebase-android-succeed'
                                $android_total = Get-Meta-Data -variable_name 'firebase-android-total'
                                @{
                                    title = "Android Test Result"
                                    value = "``$android_succeed-$android_total``"
                                    short = "true"
                                }
                            }
                            if ($ios_autotest -eq "1") {                                
                                $ios_succeed = Get-Meta-Data -variable_name 'firebase-ios-succeed'
                                $ios_total = Get-Meta-Data -variable_name 'firebase-ios-total'
                                @{
                                    title = "iOS Test Result"
                                    value = "``$ios_succeed-$ios_total``"
                                    short = "true"
                                }
                            }
                            @{
                                title = "Build Message"
                                value = "``$build_message``"
                                short = "true"
                            }
                            @{
                                title = "Example Project branch"
                                value = "``$buildkite_branch``"
                                short = "true"
                            }
                            @{
                                title = "GDK branch"
                                value = "``$gdk_branch_name``"
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
            $deployment_name = Get-Meta-Data -variable_name "deployment-name-$($i+1)"
            $deployment_url = "https://console.improbable.io/projects/${project_name}/deployments/${deployment_name}/overview"
            $deployment_button = @{
                                type = "button"
                                text = ":cloud: Deployment $deployment_name"
                                url = "$deployment_url"
                                style = "primary"
                            }
            $json_message["attachments"][0]["actions"] += ($deployment_button)
        }
    }

    $json_request = $json_message | ConvertTo-Json -Depth 10
    Write-Output $json_request
    Invoke-WebRequest -UseBasicParsing "$slack_webhook_url" -ContentType "application/json" -Method POST -Body "$json_request"
Finish-Event "slack-notify" "slack-notify"
