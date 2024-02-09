# Creates the .vsix package and publishes to your test publisher.
param(
    [Parameter(Mandatory=$true)]
    [string]
    $publisherName,

    [Parameter(Mandatory=$true)]
    [string]
    $extensionId
)

# Personal Access Token used to access the publisher account.
# This is requested the first time the script is run and is stored encrypted.
$marketplacePatLocation = "$PSScriptRoot\pat.txt"

function Set-MarketplacePat()
{
    Read-Host -Prompt "PAT" -AsSecureString | ConvertFrom-SecureString | Set-Content -Path $marketplacePatLocation
}

function Get-MarketplacePat()
{
    if (!(Test-Path $marketplacePatLocation))
    {
        Set-MarketplacePat
    }

    $securePassword = Get-Content $marketplacePatLocation | ConvertTo-SecureString

    # On Powershell >= 7
    # $securePassword | ConvertFrom-SecureString -AsPlainText

    $credential = New-Object System.Management.Automation.PSCredential ("dummy", $securePassword)
    return $credential.GetNetworkCredential().Password
}

function Update-TaskVersions()
{
    foreach ($path in $(Get-ChildItem -Recurse -Depth 1 -Filter 'task.json')) {
        $taskJson = Get-Content $path.FullName -Raw | ConvertFrom-Json
        $taskJson.version.Patch += 1
        $taskJson | ConvertTo-Json -Depth 10 | Set-Content $path.FullName
    }
}

Push-Location $taskSrcRoot
Update-TaskVersions
& $PSScriptRoot/build.ps1 Build
tfx extension publish --manifests vss-extension.json --publisher $publisherName --extension-id $extensionId --rev-version --token $(Get-MarketplacePat)
Pop-Location
