#pragma once

#include<windows.h>
#include <TraceLoggingProvider.h>
#include "msixmgrTelemetry.hpp"
#include <string>
#include "..\msixmgrLib\GeneralUtil.hpp"
using namespace MsixCoreLib;

TRACELOGGING_DECLARE_PROVIDER(g_MsixMgrTelemetryProvider);

namespace msixmgr
{
    inline void TraceLogSession(const wchar_t* WorkflowID, const wchar_t* SourceApplicationID, const wchar_t* CorrelationID)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "Session",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(SourceApplicationID, "SourceApplicationID"),
            TraceLoggingWideString(CorrelationID, "CorrelationID"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogWorkflow(const wchar_t* WorkflowID, const wchar_t* WorkflowType,
        const bool IsSuccessful, const double ExecutionTime, const wchar_t* ErrorCode, const wchar_t* ErrorDesc)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "Workflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(WorkflowType, "WorkflowType"),
            TraceLoggingBool(IsSuccessful, "IsSuccessful"),
            TraceLoggingFloat64(ExecutionTime, "ExecutionTime"),
            TraceLoggingWideString(ErrorCode, "ErrorCode"),
            TraceLoggingWideString(ErrorDesc, "ErrorDesc"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogAddWorkflow(const wchar_t* WorkflowID, const wchar_t* PackageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "AddWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(PackageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogRemoveWorkflow(const wchar_t* WorkflowID, const wchar_t* PackageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "RemoveWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(PackageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogFindWorkflow(const wchar_t* WorkflowID, const wchar_t* PackageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "FindWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(PackageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogUnpackWorkflow(const wchar_t* WorkflowID, const wchar_t* PackageName, const wchar_t* OutputImageType,
        const bool IsCreate, const bool IsApplyACLs)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "UnpackWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(PackageName, "PackageName"),
            TraceLoggingWideString(OutputImageType, "OutputImageType"),
            TraceLoggingBool(IsCreate, "IsCreate"),
            TraceLoggingBool(IsApplyACLs, "IsApplyACLs"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogApplyACLsWorkflow(const wchar_t* WorkflowID, const wchar_t* PackageName)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "ApplyACLsWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(PackageName, "PackageName"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogMountWorkflow(const wchar_t* WorkflowID, const wchar_t* MountImageType)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "MountWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(MountImageType, "MountImageType"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline void TraceLogUnmountWorkflow(const wchar_t* WorkflowID, const wchar_t* UnmountImageType)
    {
        TraceLoggingWrite(
            g_MsixMgrTelemetryProvider,
            "UnmountWorkflow",
            TraceLoggingWideString(WorkflowID, "WorkflowID"),
            TraceLoggingWideString(UnmountImageType, "UnmountImageType"),
            TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage),
            TraceLoggingKeyword(MICROSOFT_KEYWORD_CRITICAL_DATA));
    }

    inline std::string GuidToString(GUID* guid) {
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

    inline std::wstring CreateWorkflowId() {
        GUID guid;
        HRESULT hr = CoCreateGuid(&guid);

        std::wstring WorkflowId = utf8_to_utf16(GuidToString(&guid));

        return WorkflowId;
    }

    inline double CalcWorkflowElapsedTime(LARGE_INTEGER MsixMgrLoad_StartCounter,
        LARGE_INTEGER MsixMgrLoad_EndCounter, LARGE_INTEGER MsixMgrLoad_Frequency)
    {
        double WorkflowElapsedTime = (MsixMgrLoad_EndCounter.QuadPart - MsixMgrLoad_StartCounter.QuadPart) / (double)MsixMgrLoad_Frequency.QuadPart;

        return WorkflowElapsedTime;
    }

    inline std::wstring GetErrorCodeFromHRESULT(HRESULT hr)
    {
        std::stringstream ErrorCodeStream;
        ErrorCodeStream << "0x" << std::hex << hr;
        std::wstring ErrorCode = utf8_to_utf16(ErrorCodeStream.str());

        return ErrorCode;
    }

    inline std::wstring ExtractPackageNameFromFilePath(std::wstring FilePath)
    {
        size_t pos = FilePath.find_last_of('\\');
        std::wstring PackageNameExtracted = FilePath.substr(pos + 1);

        return PackageNameExtracted;
    }
}