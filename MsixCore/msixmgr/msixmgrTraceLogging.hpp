#pragma once

#include<windows.h>
#include <TraceLoggingProvider.h>
#include "msixmgrTelemetry.hpp"
#include <string>
#include "..\msixmgrLib\GeneralUtil.hpp"
using namespace MsixCoreLib;

TRACELOGGING_DECLARE_PROVIDER(g_MsixMgrTelemetryProvider);

namespace msixmgrTraceLogging
{
    void TraceLogSession(
        const wchar_t* workflowID,
        const wchar_t* sourceApplicationID,
        const wchar_t* correlationID);

    void TraceLogWorkflow(
        const wchar_t* workflowID,
        const wchar_t* workflowType,
        const bool isSuccessful,
        const double executionTime,
        const wchar_t* errorCode,
        const wchar_t* errorDesc);

    void TraceLogAddWorkflow(
        const wchar_t* workflowID,
        const wchar_t* packageName);

    void TraceLogRemoveWorkflow(
        const wchar_t* workflowID,
        const wchar_t* packageName);

    void TraceLogFindWorkflow(
        const wchar_t* workflowID,
        const wchar_t* packageName);

    void TraceLogUnpackWorkflow(
        const wchar_t* workflowID,
        const wchar_t* packageName,
        const wchar_t* outputImageType,
        const ULONGLONG outputImageSize,
        const bool isCreate,
        const bool isApplyACLs);

    void TraceLogApplyACLsWorkflow(
        const wchar_t* workflowID,
        const wchar_t* packageName);

    void TraceLogMountWorkflow(
        const wchar_t* workflowID,
        const wchar_t* mountImageType);

    void TraceLogUnmountWorkflow(
        const wchar_t* workflowID,
        const wchar_t* unmountImageType);

    std::string GuidToString(
        GUID* guid);

    std::wstring CreateWorkflowId();

    double CalcWorkflowElapsedTime(
        LARGE_INTEGER msixMgrLoad_StartCounter,
        LARGE_INTEGER msixMgrLoad_EndCounter,
        LARGE_INTEGER msixMgrLoad_Frequency);

    std::wstring GetErrorCodeFromHRESULT(
        HRESULT hr);

    std::wstring ExtractPackageNameFromFilePath(
        std::wstring filePath);
}