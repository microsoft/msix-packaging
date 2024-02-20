<#
.SYNOPSIS
    This script invokes AppAttachFramwork DLL to perform App Attach

.DESCRIPTION
    This script triggers AppAttach framework DLL with required input json configuration to porform App Attach.

.PARAMETER inputJsonStr
    Specifies input json configuration to trigger AppAttach Framework DLL with for AppAttach.

.PARAMETER dllPath
    Specifies the target DLL path used for AppAttach flow.
#>

param([string]$inputJsonStr, [string]$dllPath)

# Load the required DLLs
Add-Type -Path (Join-Path $dllPath "AppAttachKernel.dll")
Add-Type -Path (Join-Path $dllPath "AppAttachMessenger.dll")

# Define the C# class that implements IMessageHandler
$source = @"
using AppAttachMessenger.Interface;
using AppAttachMessenger;

public class AzureDevOpsMessageHandler : IMessageHandler {
    public string HandleMessage(Message message) {
        System.Console.WriteLine(message.Content);
        return message.Content;
    }
}
"@

# Add the C# class to the current session
Add-Type -TypeDefinition $source -ReferencedAssemblies (Join-Path $dllPath "AppAttachMessenger.dll")

# Function to get or create the singleton instance of AzureDevOpsMessageHandler
function Get-MessageHandlerSingleton {
    if (-not $script:messageHandler) {
        $script:messageHandler = New-Object AzureDevOpsMessageHandler
    }
    return $script:messageHandler
}

# Create and invoke AppAttachKernel object with configured json
$custObj = New-Object AppAttachKernel.AppAttachKernelController($inputJsonStr)
$token = New-Object 'System.Threading.CancellationToken'

# Get or create singleton instance of the AzureDevOpsMessageHandler
$messageHandler =  Get-MessageHandlerSingleton

$appAttachFlowResponse = $custObj.execute($token, $messageHandler)

if ($appAttachFlowResponse.isError())
{
    throw (($appAttachFlowResponse.getAppAttachOutputs())[0]).getMessage()
}
