[CmdletBinding()]
param([string]$PreferredVersion, [string]$Architecture)

# The MSBuild helpers need the VSTS Task SDK.
# Import it before so it is available.
Import-Module $PSScriptRoot/ps_modules/VstsTaskSdk
Import-Module $PSScriptRoot/MSBuildHelpers/MSBuildHelpers

return Select-MSBuildPath -PreferredVersion $PreferredVersion -Architecture $Architecture 3>$null 6>$null