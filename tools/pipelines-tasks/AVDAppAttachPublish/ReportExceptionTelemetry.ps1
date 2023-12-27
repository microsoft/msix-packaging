<#
.SYNOPSIS
    This script Reports Exceptions to AppAttach Telemetry

.DESCRIPTION
    This script triggers AppAttachTelemetry DLL with required parameters to capture exceptions in AppAttach Telemetry

.PARAMETER exceptionMessage
    Specifies the exception message to be captured

.PARAMETER targetDLL
    Specifies the target DLL to be triggered for AppAttach Telemetry.
#>

param([string]$exceptionMessage, [string]$targetDLL, [string]$clientType, [string]$clientVersion)

try
{
    Add-Type -Path $targetDLL

    # Create and invoke AppAttachTelemetry object.
    $telemetryProvider = [AppAttachTelemetry.TelemetryProvider]::s_instance

    $appAttachExceptionBuilderObj =  New-Object AppAttachTelemetry.Builders.AppAttachExceptionsBuilder
    $appAttachExceptionBuilderObj.SetIsSuccessful("False") | Out-Null
    $appAttachExceptionBuilderObj.SetErrorCode($exceptionMessage) | Out-Null
    $appAttachExceptionBuilderObj.SetErrorDesc($exceptionMessage) | Out-Null
    $appAttachExceptionBuilderObj.SetClientType($clientType) | Out-Null
    $appAttachExceptionBuilderObj.SetClientVersion($clientVersion) | Out-Null
    $appAttachExceptionBuilderObj.SetState("ErrorOccurred") | Out-Null
    $appAttachExceptions = $appAttachExceptionBuilderObj.Build()

    $telemetryProvider.ReportException($appAttachExceptions)
}
catch
{
    Write-Error "Error Occurred:"
    Write-Error $_
}

