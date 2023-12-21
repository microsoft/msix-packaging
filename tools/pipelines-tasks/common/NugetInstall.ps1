<#
.SYNOPSIS
    This script installs the provided nuget package using nuget CLI command

.DESCRIPTION
    This script installs provided nuget package with given packageId and version

.PARAMETER packageId
    Nuget package Id that needs to be installed

.PARAMETER version
    Nuget Package Id version that needs to be installed

.PARAMETER outputDirectory
    Output path where provide nuget package has to be installed

#>

param([string]$packageId=$null, [string]$version=$null, [string]$outputDirectory)
    if ($packageId -and $version)
    {
        & nuget install $packageId -Version $version -NonInteractive -OutputDirectory $outputDirectory
    }
    else
    {
        & nuget install 'packaes.config' -NonInteractive -OutputDirectory $outputDirectory
        throw "custom exception throwing"
    }

