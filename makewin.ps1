[CmdletBinding()]
param(
    [parameter()]
    [ValidateSet(, "64-bit", "32-bit")]
    [string]$Architecture = "64-bit"
)
function Set-VsDevCmd {
    <#
    .SYNOPSIS
        Finds the VSDevCmd // VCVars64 scripts and adds the params to the environment in powershell
    .PARAMETER Architecture
        Set your system architecture to either 32-Bit or 64-Bit
    .NOTES
        Kudos https://stackoverflow.com/questions/2124753/how-can-i-use-powershell-with-the-visual-studio-command-prompt
    #>
    [CmdletBinding()]
    Param(
        [parameter()]
        [ValidateSet(, "64-bit", "32-bit")]
        [string]$Architecture = "64-bit"
    )   
    
    switch ($Architecture) {
        "64-bit" {
            $path = "C:\program files*\Microsoft Visual Studio*\2017\*\VC\Auxiliary\Build\vcvars64.bat"
            break
        }
        "32-bit" {
            $path = "C:\program files*\Microsoft Visual Studio*\2017\*\Common7\Tools\VsDevCmd.bat"
        }
    }
    $vsDevPath = (Get-ChildItem -Path $path).FullName
    Push-Location (Split-Path $vsDevPath -Parent)
    $pathVars = @()
    cmd /c "$(Split-Path $vsDevPath -leaf)&set" |
        ForEach-Object {
        if ($_ -match "=") {
            $v = $_.Split("=")
            $tmpHash = @{}
            $tmpHash.$($v[0]) = $($v[1])
            $pathVars += $tmpHash
            set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
        }
    }
    Pop-Location
    write-host "`Visual Studio Developer Command Prompt variables set." -ForegroundColor Yellow
    return $PathVars
}

$scriptpath = $MyInvocation.MyCommand.Path
if ($scriptpath) {
    $fp = split-path $scriptpath
}
if (!(Test-Path "$fp\.vs")) {
    Write-Host "Creating .vs folder in $fp" -ForegroundColor Green
    New-Item -Path "$fp" -Name ".vs" -ItemType Directory -Force | out-null
}
if (Test-Path "$fp\CMakeFiles") {
    Write-Host "Deleting CMakeFiles in $fp" -ForegroundColor Green
    Remove-Item -Path "$fp\CMakeFiles" -Recurse -Force | out-null
}
if (Test-Path "$fp\CMakeCache.txt") {
    Write-Host "Deleting CMakeCache.txt in $fp" -ForegroundColor Green
    Remove-Item -Path "$fp\CMakeCache.txt" -Force | out-null
}

Write-Host "Loading Visual Studio Developer Command Prompt Variables.."
Set-VsDevCmd -Architecture $Architecture | out-Null
cmake -DWIN32=on -DCMAKE_BUILD_TYPE=MinSizeRel -G "NMake Makefiles"
nmake