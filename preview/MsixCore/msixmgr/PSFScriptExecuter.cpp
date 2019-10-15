#include <windows.h>

#include "PSFScriptExecuter.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"

using namespace MsixCoreLib;

const PCWSTR PSFScriptExecuter::HandlerName = L"PSFScriptExecuter";

HRESULT PSFScriptExecuter::ExecuteForAddRequest()
{
    if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
    {
        return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
    }

    // Run the script


    return S_OK;
}

HRESULT PSFScriptExecuter::ExecuteForRemoveRequest()
{
    return S_OK;
}

HRESULT PSFScriptExecuter::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<PSFScriptExecuter> localInstance(new PSFScriptExecuter(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
