# Note: This script needs to be run with admin privileges.

param
(
    # Must be a full path
    [Parameter(Mandatory=$true)]
    [string]$vhdxPath,

    # Size in MB.
    [Parameter(Mandatory=$true)]
    [int]$vhdxSize,

    [Parameter(Mandatory=$true)]
    [string]$msixPackagePath,

    [Parameter(Mandatory=$true)]
    [string]$msixmgrPath
)

$ErrorActionPreference = "Stop"
$PSDefaultParameterValues['*:Encoding'] = 'utf8'

Remove-Item $vhdxPath -ErrorAction SilentlyContinue

# Create parent directory
New-Item -ItemType Directory -Path (Split-Path $vhdxPath) -ErrorAction SilentlyContinue

# Create the disk
# Note that the docs for creating a VHDX for app attach say to use
#   New-VHD -SizeBytes $vhdxSize -Path $vhdxPath -Dynamic -Confirm:$false
# but New-VHD is not available on the build agents.
# Instead we use diskpart.
# diskpart is an interactive program. We pass it a script of what we want to do.
# We could also pass it through the standard input.
$scriptFile = Join-Path $env:TEMP 'create-vhdx.txt'
$diskpartCommand = "create vdisk file=`"$vhdxPath`" maximum=$vhdxSize"
Write-Host "diskpart command: $diskpartCommand"
$diskpartCommand | Out-File $scriptFile

try
{
    Start-Process -Wait diskpart /s,$scriptFile
    if (-not $?)
    {
        throw "Diskpart failed to create new virtual disk."
    }

    # Mount the disk
    $vhdxObject = Mount-DiskImage $vhdxPath -Passthru
    if (-not $?)
    {
        throw "Mounting virtual disk failed."
    }

    # Format the disk
    $vhdxObject | Format-List
    $disk = Initialize-Disk -Passthru -Number $vhdxObject.Number -PartitionStyle MBR
    $partition = New-Partition -AssignDriveLetter -UseMaximumSize -DiskNumber $disk.Number
    Format-Volume -FileSystem NTFS -Confirm:$false -DriveLetter $partition.DriveLetter -Force

    # Create a directory to hold the app
    $vhdxRoot = $partition.DriveLetter + ':\VHDXRoot'
    New-Item -Path $vhdxRoot -ItemType Directory

    # Unpack the app to the app into the distk
    & $msixmgrPath -Unpack -packagePath $msixPackagePath -destination $vhdxRoot -applyacls
}
finally
{
    # Remove-Item $scriptFile
    Dismount-DiskImage $vhdxPath -ErrorAction SilentlyContinue
}