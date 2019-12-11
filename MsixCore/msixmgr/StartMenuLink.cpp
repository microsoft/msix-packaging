#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>
#include <propvarutil.h>
#include <propkey.h>

#include "FilePaths.hpp"
#include "StartMenuLink.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "MsixTraceLoggingProvider.hpp"

using namespace MsixCoreLib;

const PCWSTR StartMenuLink::HandlerName = L"StartMenuLink";

HRESULT StartMenuLink::CreateLink(ExecutionInfo* executionInfo, PCWSTR linkFilePath, PCWSTR description, PCWSTR appUserModelId)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Creating Link",
        TraceLoggingValue(executionInfo->resolvedExecutableFilePath.c_str(), "ExecutableFilePath"),
        TraceLoggingValue(executionInfo->commandLineArguments.c_str(), "Arguments"),
        TraceLoggingValue(executionInfo->workingDirectory.c_str(), "WorkingDirectory"),
        TraceLoggingValue(linkFilePath, "LinkFilePath"),
        TraceLoggingValue(appUserModelId, "AppUserModelId"));

    ComPtr<IShellLink> shellLink;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<LPVOID*>(&shellLink)));
    RETURN_IF_FAILED(shellLink->SetPath(executionInfo->resolvedExecutableFilePath.c_str()));
    RETURN_IF_FAILED(shellLink->SetArguments(executionInfo->commandLineArguments.c_str()));
    if (!executionInfo->workingDirectory.empty())
    {
        RETURN_IF_FAILED(shellLink->SetWorkingDirectory(executionInfo->workingDirectory.c_str()));
    }

    if (appUserModelId != NULL && appUserModelId[0] != 0)
    {
        ComPtr<IPropertyStore> propertyStore;
        PROPVARIANT appIdPropVar;
        RETURN_IF_FAILED(shellLink->QueryInterface(IID_IPropertyStore, reinterpret_cast<LPVOID*>(&propertyStore)));
        RETURN_IF_FAILED(InitPropVariantFromString(appUserModelId, &appIdPropVar));
        RETURN_IF_FAILED(propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar));
        RETURN_IF_FAILED(propertyStore->Commit());
        PropVariantClear(&appIdPropVar);
    }

    RETURN_IF_FAILED(shellLink->SetDescription(description));

    ComPtr<IPersistFile> persistFile;
    RETURN_IF_FAILED(shellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&persistFile)));
    RETURN_IF_FAILED(persistFile->Save(linkFilePath, TRUE));
    return S_OK;
}

HRESULT StartMenuLink::ExecuteForAddRequest()
{
    if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
    {
        m_msixRequest->GetMsixResponse()->SetErrorStatus(HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT), L"User cancelled installation.");
        return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
    }
    auto packageInfo = m_msixRequest->GetPackageInfo();

    std::wstring filePath = FilePathMappings::GetInstance().GetMap()[L"Common Programs"] + L"\\" + packageInfo->GetDisplayName() + L".lnk";
    std::wstring appUserModelId = m_msixRequest->GetPackageInfo()->GetId();
    RETURN_IF_FAILED(CreateLink(packageInfo->GetExecutionInfo(), filePath.c_str(), L"", appUserModelId.c_str()));

    return S_OK;
}

HRESULT StartMenuLink::ExecuteForRemoveRequest()
{
    auto packageInfo = m_msixRequest->GetPackageInfo();

    std::wstring filePath = FilePathMappings::GetInstance().GetMap()[L"Common Programs"] + L"\\" + packageInfo->GetDisplayName() + L".lnk";

    RETURN_IF_FAILED(DeleteFile(filePath.c_str()));
    return S_OK;
}

HRESULT StartMenuLink::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<StartMenuLink> localInstance(new StartMenuLink(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
