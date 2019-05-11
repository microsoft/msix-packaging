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
    //copy rest of the keys here
    AutoPtr<RegistryDevirtualizer> registryDevirtualizer;
    RETURN_IF_FAILED(registryDevirtualizer->Run(false));
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

//TODO: Unload mounted hive here
/*WriteDevirtualizedRegistry::~WriteDevirtualizedRegistry()
{
    // unload loaded hive here

}*/