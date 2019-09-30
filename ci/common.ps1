function Write-Log() {
  param(
    [string] $msg,
    [Parameter(Mandatory=$false)] [bool] $expand = $false
  )
  if ($expand) {
      Write-Output "+++ $($msg)"
  } else {
      Write-Output "--- $($msg)"
  }
}

function Start-Event() {
    param(
        [string] $event_name,
        [string] $event_parent
    )

    # Start this tracing span.
    Start-Process -NoNewWindow "imp-ci" -ArgumentList @(`
        "events", "new", `
        "--name", "$($event_name)", `
        "--child-of", "$($event_parent)"
    ) | Out-Null

    Write-Log "--- $($event_name)"
}

function Finish-Event() {
    param(
        [string] $event_name,
        [string] $event_parent
    )

    # Emit the end marker for this tracing span.
    Start-Process -NoNewWindow "imp-ci"  -ArgumentList @(`
        "events", "new", `
        "--name", "$($event_name)", `
        "--child-of", "$($event_parent)"
    ) | Out-Null
}

## Checks whether the specified environment variable has been set. If it has, return its value. Else return the default value.
function Get-Env-Variable-Value-Or-Default() {
    param(
        [string] $environment_variable_name,
        [string] $default_value
    )

    If (Test-Path env:$environment_variable_name) {
        $environment_variable_value = Get-Content -Path env:$environment_variable_name
        return $environment_variable_value
    } Else {
        return $default_value
    }
}

$ErrorActionPreference = 'Stop'