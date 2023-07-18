#pragma once

#include<windows.h>
#include <TraceLoggingProvider.h>
#include "msixmgrTelemetry.hpp"
#include <string>
#include "..\msixmgrLib\GeneralUtil.hpp"
#include "msixmgrTraceLogging.hpp"
#include <sstream>
using namespace msixmgrTraceLogging;
using namespace MsixCoreLib;

TRACELOGGING_DECLARE_PROVIDER(g_MsixMgrTelemetryProvider);

// Purpose:
// - This file contains all the TraceLogging statements which will perform the task for
//   actual telemetry logging. All the TraceLogging Functions are being called from msixmgr.cpp file.
// - It also contains Helper Functions for Creating WorkflowID, Calculating Workflow Elapsed Time,
//   Getting Error Code from HRESULT, Extracting PackageName from Filepath, etc.

namespace msixmgrTraceLogging
{
    void TraceLogSession(const wchar_t* workflowID, const wchar_t* sourceApplicationID, const wchar_t* correlationID)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "Session",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(sourceApplicationID, "SourceApplicationID"),
            TraceLoggingWideString(correlationID, "CorrelationID"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogWorkflow(const wchar_t* workflowID, const wchar_t* workflowType,
        const bool isSuccessful, const double executionTime, const wchar_t* errorCode, const wchar_t* errorDesc)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "Workflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(workflowType, "WorkflowType"),
            TraceLoggingBool(isSuccessful, "IsSuccessful"),
            TraceLoggingFloat64(executionTime, "ExecutionTime"),
            TraceLoggingWideString(errorCode, "ErrorCode"),
            TraceLoggingWideString(errorDesc, "ErrorDesc"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogAddWorkflow(const wchar_t* workflowID, const wchar_t* packageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "AddWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(packageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogRemoveWorkflow(const wchar_t* workflowID, const wchar_t* packageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "RemoveWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(packageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogFindWorkflow(const wchar_t* workflowID, const wchar_t* packageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "FindWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(packageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogUnpackWorkflow(const wchar_t* workflowID, const wchar_t* packageName, const wchar_t* outputImageType,
        const ULONGLONG outputImageSize, const bool isCreate, const bool isApplyACLs)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "UnpackWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(packageName, "PackageName"),
            TraceLoggingWideString(outputImageType, "OutputImageType"),
            TraceLoggingInt64(outputImageSize, "OutputImageSize"),
            TraceLoggingBool(isCreate, "IsCreate"),
            TraceLoggingBool(isApplyACLs, "IsApplyACLs"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogApplyACLsWorkflow(const wchar_t* workflowID, const wchar_t* packageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "ApplyACLsWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(packageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogMountWorkflow(const wchar_t* workflowID, const wchar_t* mountImageType)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "MountWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(mountImageType, "MountImageType"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    void TraceLogUnmountWorkflow(const wchar_t* workflowID, const wchar_t* unmountImageType)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "UnmountWorkflow",
            TraceLoggingWideString(workflowID, "WorkflowID"),
            TraceLoggingWideString(unmountImageType, "UnmountImageType"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    std::string GuidToString(GUID* guid) {
        char guid_string[37];
        snprintf(
            guid_string, sizeof(guid_string),
            "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid->Data1, guid->Data2, guid->Data3,
            guid->Data4[0], guid->Data4[1], guid->Data4[2],
            guid->Data4[3], guid->Data4[4], guid->Data4[5],
            guid->Data4[6], guid->Data4[7]);
        return guid_string;
    }

    std::wstring CreateWorkflowId() {
        GUID guid;
        HRESULT hr = CoCreateGuid(&guid);

        std::wstring workflowId = utf8_to_utf16(GuidToString(&guid));

        return workflowId;
    }

    double CalcWorkflowElapsedTime(LARGE_INTEGER msixMgrLoad_StartCounter,
        LARGE_INTEGER msixMgrLoad_EndCounter, LARGE_INTEGER msixMgrLoad_Frequency)
    {
        double workflowElapsedTime = (msixMgrLoad_EndCounter.QuadPart - msixMgrLoad_StartCounter.QuadPart) / (double)msixMgrLoad_Frequency.QuadPart;

        return workflowElapsedTime;
    }

    std::wstring GetErrorCodeFromHRESULT(HRESULT hr)
    {
        std::stringstream errorCodeStream;
        errorCodeStream << "0x" << std::hex << hr;
        std::wstring errorCode = utf8_to_utf16(errorCodeStream.str());

        return errorCode;
    }

    std::wstring ExtractPackageNameFromFilePath(std::wstring filePath)
    {
        size_t pos = filePath.find_last_of('\\');
        std::wstring packageNameExtracted = filePath.substr(pos + 1);

        return packageNameExtracted;
    }
}