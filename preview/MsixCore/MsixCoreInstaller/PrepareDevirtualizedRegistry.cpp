#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>
#include <experimental/filesystem> // C++-standard header file name

#include "PrepareDevirtualizedRegistry.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "Constants.hpp"
#include "RegistryDevirtualizer.hpp"

using namespace MsixCoreLib;

const PCWSTR PrepareDevirtualizedRegistry::HandlerName = L"PrepareDevirtualizedRegistry";

HRESULT PrepareDevirtualizedRegistry::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(ExtractRegistry(false));
    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::ExtractRegistry(bool remove)
{
    std::wstring registryFilePath = m_msixRequest->GetPackageDirectoryPath() + registryDatFile;

    AutoPtr<RegistryDevirtualizer> registryDevirtualizer;
    RETURN_IF_FAILED(RegistryDevirtualizer::Create(registryFilePath, m_msixRequest, &registryDevirtualizer));
    //TODO: This should run only for remove, not add(move to remove method)
    //RETURN_IF_FAILED(registryDevirtualizer->Run(remove));
    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::ExecuteForRemoveRequest()
{
    HRESULT hrRemoveRegistry = ExtractRegistry(true);
    if (FAILED(hrRemoveRegistry))
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "Unable to remove registry",
            TraceLoggingLevel(WINEVENT_LEVEL_WARNING),
            TraceLoggingValue(hrRemoveRegistry, "HR"));
    }

    return S_OK;
}

HRESULT PrepareDevirtualizedRegistry::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<PrepareDevirtualizedRegistry > localInstance(new PrepareDevirtualizedRegistry(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}