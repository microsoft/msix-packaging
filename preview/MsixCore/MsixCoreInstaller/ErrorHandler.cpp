#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "ErrorHandler.hpp"
#include "MsixRequest.hpp"
using namespace MsixCoreLib;

const PCWSTR ErrorHandler::HandlerName = L"ErrorHandler";

HRESULT ErrorHandler::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(RemovePackage(m_msixRequest->GetPackageInfo()->GetPackageFullName()));
    return S_OK;
}

HRESULT ErrorHandler::RemovePackage(std::wstring packageFullName)
{
    AutoPtr<MsixRequest> removePackageRequest;
    RETURN_IF_FAILED(MsixRequest::Make(OperationType::Remove, std::wstring(), packageFullName,
        MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &removePackageRequest));

    const HRESULT hrCancelRequest = removePackageRequest->ProcessRequest();
    if (FAILED(hrCancelRequest))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Failed to process cancel request",
        TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
        TraceLoggingValue(hrCancelRequest, "HR"));
    }
    return S_OK;
}

HRESULT ErrorHandler::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<ErrorHandler> localInstance(new ErrorHandler(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}