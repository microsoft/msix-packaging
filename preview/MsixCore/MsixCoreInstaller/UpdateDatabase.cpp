#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"
#include "UpdateDatabase.hpp"
#include "MsixRequest.hpp"
#include "Database.hpp"

using namespace MsixCoreLib;

const PCWSTR UpdateDatabase::HandlerName = L"UpdateDatabase";


HRESULT UpdateDatabase::ExecuteForAddRequest()
{
    RETURN_IF_FAILED(Database::AddPackageForCurrentUser(m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str()));

    return S_OK;
}

HRESULT UpdateDatabase::ExecuteForRemoveRequest()
{
    RETURN_IF_FAILED(Database::RemovePackageForCurrentUser(m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str()));

    return S_OK;
}

HRESULT UpdateDatabase::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<UpdateDatabase> localInstance(new UpdateDatabase(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    *instance = localInstance.release();

    return S_OK;
}