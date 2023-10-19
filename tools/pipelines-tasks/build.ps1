[CmdletBinding()]
param (
    [string]
    [ValidateSet(
        'InstallDevelopmentTools',
        'InstallDependencies',
        'BuildCommonHelpers',
        'Build',
        'BuildForProduction'
    )]
    $action
)

$taskNames = (
    "AppInstallerFile",
    "AVDAppAttachPublish",
    "MsixAppAttach",
    "MsixPackaging",
    "MsixSigning"
)

# npm likes writing to the stderr, which PS will sometimes interpret as an error.
# This wrapper lets us call it while temporarily disabling this error handling.
function npm
{
    $oldErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'SilentlyContinue'
    npm.cmd $args
    $ErrorActionPreference = $oldErrorActionPreference

    if ($LASTEXITCODE -ne 0)
    {
        throw "npm failed"
    }
}

function OnDirectory([string]$dir, [scriptblock]$script, [object[]]$arguments)
{
    Push-Location $dir
    try
    {
        Invoke-Command $script -ArgumentList $arguments
    }
    finally
    {
        Pop-Location
    }
}

function OnEachTask([string]$message, [scriptblock]$script)
{
    foreach ($task in $taskNames)
    {
        Write-Host "$task : $message"
        OnDirectory "$PSScriptRoot\$task" $script
    }
}

# Builds the common helpers and installs it to each task.
# This needs to run after any changes to the common helpers.
function BuildCommonHelpers([switch]$installDependencies)
{
    OnDirectory "$PSScriptRoot\common" -arguments $installDependencies {
        param($installDependencies)

        if ($installDependencies)
        {
            Write-Host "Installing dependencies"
            npm ci
        }

        # Build the directory
        Write-Host "Compiling common helpers"
        npx tsc
        if (-not $?)
        {
            throw "Failed to build 'common'"
        }

        # Pack the package for npm.
        # We need to reference a .tgz package instead of a directory because
        # tfx cannot deal with symbolic links.
        #
        # I.e. if we reference the package in the task's package.json as
        #   "dependencies": {
        #     "common": "../common"
        #   }
        # then the task's node_modules/common will be a symbolic link to
        # /common, and creating the extension with tfx will fail.
        #
        # Instead we reference the packaged common as
        #   "dependencies": {
        #     "common": "../common/msix-tasks-helpers-1.0.0.tgz"
        #   }
        # This will copy and extract the package into the task's
        # node_modules/.
        #
        # The downside of this is that it changes to common/ are not
        # picked up by tasks, hence this function.

        # Before packaging, remove existing packages so they are not
        # included in new package.
        Remove-Item *.tgz

        Write-Host "Packaging common helpers"
        npm pack

        # Install the package to all the tasks.
        # The helpers need to be installed to each task because each task is
        # installed independently to the agent, so only files on the task's
        # directory can be used.
        OnEachTask "Installing common helpers" {
            npm install common
        }
    }
}

# Installs the development tools required to work on the repo.
# This needs Node.js v10 to already be installed, and will only install the
# required global Node modules
function InstallDevelopmentTools()
{
    # Typescript compiler
    npm install -g typescript

    # CLI tools to interact with Azure DevOps (e.g. build and publish the extension)
    npm install -g tfx-cli

    # Test platform
    npm install -g mocha
}

# Installs the dependencies for every project (the common helpers and all tasks)
function InstallAllDepenencies()
{
    # Always prefer 'npm ci' over 'npm install' to use specific package versions
    # from package-lock.json and get the same results on any machine.

    # First build the common helpers as they are a dependency of everything else.
    BuildCommonHelpers -installDependencies

    # Install dependencies for top level scripts.
    OnDirectory $PSScriptRoot {
        npm ci
    }

    # Install dependencies for each task.
    # This will install the common helpers again, but it's not too much extra work.
    OnEachTask "Install dependencies" {
        npm ci
    }

    OnDirectory $PSScriptRoot\MsixPackaging {
        if (-not (Test-Path ps_modules/VstsTaskSdk))
        {
            Write-Host "Installing VSTS Task SDK Powershell Module"
            New-Item -Type Directory ps_modules -ErrorAction SilentlyContinue
            Save-Module -Name VstsTaskSdk -Path .\ps_modules\
        }
    }

    # Create a test certificate for local testing/debugging.
    # This creates the file expected by the tests and debug inputs.
    # Create the certificate
    $cert = New-SelfSignedCertificate -Type Custom `
        -Subject "CN=HelloWorldPublisher" `
        -KeyUsage DigitalSignature `
        -FriendlyName "HelloWorldPublisher" `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}")

    # Export it as a .pfx
    $password = ConvertTo-SecureString -String password -Force -AsPlainText
    Export-PfxCertificate -Cert $cert -FilePath $PSScriptRoot\test\assets\certificate.pfx -Password $password

    # Write it as a base64 string
    $certBytes = $cert.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Pfx)
    [System.Convert]::ToBase64String($certBytes) | Out-File $PSScriptRoot\test\assets\certificate.txt -Encoding utf8

    # Remove the certificate from the cert store
    $certPath = "Cert:\CurrentUser\My\$($cert.Thumbprint)"
    Remove-Item $certPath
}

# Build all the tasks and update the loc strings.
function Build() {
    Write-Host "Build all directories and update loc files"
    OnDirectory $PSScriptRoot {
        npx tsc
        node create-resjson.js
    }
}

# Build all the tasks and removes development dependencies.
# This doesn't create the extension .vsix package.
function BuildForProduction()
{
    InstallDevelopmentTools
    InstallAllDepenencies
    Build

    # Fail if loc files changed.
    # This is to cause the pipelines to fail if the changes were not commited.
    # Ignore changes to package-lock.json as it will contain a new hash for the common helpers.
    $filesModified = git status --porcelain | Where-Object { $_ -notmatch 'package-lock.json' }
    if ($filesModified)
    {
        $filesModified
        throw "There are uncommited changes to loc strings"
    }

    OnEachTask "Remove development dependencies" {
        npm prune
    }
}

switch ($action)
{
    'InstallDevelopmentTools' { InstallDevelopmentTools }
    'InstallDependencies' { InstallAllDepenencies }
    'BuildCommonHelpers' { BuildCommonHelpers }
    'Build' {Build}
    'BuildForProduction' { BuildForProduction }
}