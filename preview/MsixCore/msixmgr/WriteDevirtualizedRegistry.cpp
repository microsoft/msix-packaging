#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "WriteDevirtualizedRegistry.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"
#include "RegistryDevirtualizer.hpp"

using namespace MsixCoreLib;

const PCWSTR WriteDevirtualizedRegistry::HandlerName = L"WriteDevirtualizedRegistry";

HRESULT WriteDevirtualizedRegistry::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(m_msixRequest->GetRegistryDevirtualizer()->Run(false));
    RETURN_IF_FAILED(m_msixRequest->GetRegistryDevirtualizer()->UnloadMountedHive());
    return S_OK;
}

HRESULT WriteDevirtualizedRegistry::ExecuteForRemoveRequest()
{
    const HRESULT hrRemoveRegistry = m_msixRequest->GetRegistryDevirtualizer()->Run(true);
    if (FAILED(hrRemoveRegistry))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to remove registry",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrRemoveRegistry, "HR"));
    }
    RETURN_IF_FAILED(m_msixRequest->GetRegistryDevirtualizer()->UnloadMountedHive());
    return S_OK;
}

HRESULT WriteDevirtualizedRegistry::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<WriteDevirtualizedRegistry > localInstance(new WriteDevirtualizedRegistry(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}