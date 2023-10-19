param([string]$inputJsonStr, [string]$targetDLL)

Write-Output $inputJsonStr
Write-Output $targetDLL

$scriptDir = $PSScriptRoot

Add-Type -Path $targetDLL

#Create AppAttachKernel object
$custObj = New-Object AppAttachKernel.AppAttachKernelController($inputJsonStr)
$token = New-Object 'System.Threading.CancellationToken'

$custObj.execute($token)