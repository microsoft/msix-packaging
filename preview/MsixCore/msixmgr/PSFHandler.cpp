#include <windows.h>

#include "PSFHandler.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"

using namespace MsixCoreLib;

const PCWSTR PSFHandler::HandlerName = L"PSFHandler";

HRESULT PSFHandler::ExecuteForAddRequest()
{
    return S_OK;
}

HRESULT PSFHandler::ExecuteForRemoveRequest()
{
    return S_OK;
}

HRESULT PSFHandler::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<PSFHandler> localInstance(new PSFHandler(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}