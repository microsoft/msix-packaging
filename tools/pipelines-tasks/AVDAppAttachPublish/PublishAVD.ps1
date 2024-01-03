<#
.SYNOPSIS
    This script invokes AppAttachFramwork DLL to perform App Attach

.DESCRIPTION
    This script triggers AppAttach framework DLL with required input json configuration to porform App Attach.

.PARAMETER inputJsonStr
    Specifies input json configuration to trigger AppAttach Framework DLL with for AppAttach.

.PARAMETER targetDLL
    Specifies the target DLL to be triggered for AppAttach.
#>

param([string]$inputJsonStr, [string]$targetDLL)

Add-Type -Path $targetDLL

# Create and invoke AppAttachKernel object with configured json
$custObj = New-Object AppAttachKernel.AppAttachKernelController($inputJsonStr)
$token = New-Object 'System.Threading.CancellationToken'

$appAttachFlowResponse = $custObj.execute($token)
if ($appAttachFlowResponse.isError())
{
    throw (($appAttachFlowResponse.getAppAttachOutputs())[0]).getMessage()
}