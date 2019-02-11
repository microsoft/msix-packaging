#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "FilePaths.hpp"
#include "StartMenuLink.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>

const PCWSTR StartMenuLink::HandlerName = L"StartMenuLink";

HRESULT StartMenuLink::CreateLink(PCWSTR targetFilePath, PCWSTR linkFilePath, PCWSTR description)
{
    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Creating Link",
        TraceLoggingValue(targetFilePath, "TargetFilePath"),
        TraceLoggingValue(linkFilePath, "LinkFilePath"));

    ComPtr<IShellLink> shellLink;
    RETURN_IF_FAILED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<LPVOID*>(&shellLink)));
    RETURN_IF_FAILED(shellLink->SetPath(targetFilePath));
    RETURN_IF_FAILED(shellLink->SetDescription(description));

    ComPtr<IPersistFile> persistFile;
    RETURN_IF_FAILED(shellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<LPVOID*>(&persistFile)));
    RETURN_IF_FAILED(persistFile->Save(linkFilePath, TRUE));
    return S_OK;
}

HRESULT StartMenuLink::ExecuteForAddRequest()
{
    PackageInfo* packageInfo = m_msixRequest->GetPackageInfo();

    std::wstring filePath = m_msixRequest->GetFilePathMappings()->GetMap()[L"Common Programs"] + L"\\" + packageInfo->GetDisplayName() + L".lnk";

    std::wstring resolvedExecutableFullPath = m_msixRequest->GetFilePathMappings()->GetExecutablePath(packageInfo->GetExecutableFilePath(), packageInfo->GetPackageFullName().c_str());
    RETURN_IF_FAILED(CreateLink(resolvedExecutableFullPath.c_str(), filePath.c_str(), L""));

    return S_OK;
}

HRESULT StartMenuLink::ExecuteForRemoveRequest()
{
    PackageInfo* packageInfo = m_msixRequest->GetPackageInfo();

    std::wstring filePath = m_msixRequest->GetFilePathMappings()->GetMap()[L"Common Programs"] + L"\\" + packageInfo->GetDisplayName() + L".lnk";

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
