#include <windows.h>

#include <shlobj_core.h>
#include <CommCtrl.h>

#include "RegistryKey.hpp"
#include "AddRemovePrograms.hpp"
#include "GeneralUtil.hpp"
#include <TraceLoggingProvider.h>
#include "Constants.hpp"
using namespace Win7MsixInstallerLib;

const PCWSTR AddRemovePrograms::HandlerName = L"AddRemovePrograms";

HRESULT AddRemovePrograms::ExecuteForAddRequest()
{
    if (m_msixRequest->GetMsixResponse()->GetIsInstallCancelled())
    {
        return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
    }

    auto packageInfo = m_msixRequest->GetPackageInfo();
    std::wstring packageFullName = packageInfo->GetPackageFullName();

    RegistryKey uninstallKey;
    RETURN_IF_FAILED(uninstallKey.Open(HKEY_LOCAL_MACHINE, uninstallKeyPath.c_str(), KEY_WRITE));

    RegistryKey packageKey;
    RETURN_IF_FAILED(uninstallKey.CreateSubKey(packageFullName.c_str(), KEY_WRITE, &packageKey));

    std::wstring displayName = packageInfo->GetDisplayName();
    RETURN_IF_FAILED(packageKey.SetStringValue(L"DisplayName", displayName));

    std::wstring directoryPath = m_msixRequest->GetPackageDirectoryPath();
    RETURN_IF_FAILED(packageKey.SetStringValue(L"InstallLocation", directoryPath));

    WCHAR filePath[MAX_PATH];
    DWORD lengthCopied = GetModuleFileNameW(nullptr, filePath, MAX_PATH);
    if (lengthCopied == 0)
    {
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }

    std::wstring uninstallCommand = filePath + std::wstring(L" -RemovePackage ") + packageFullName;
    RETURN_IF_FAILED(packageKey.SetStringValue(L"UninstallString", uninstallCommand));

    RETURN_IF_FAILED(packageKey.SetStringValue(L"Publisher", packageInfo->GetPublisherDisplayName()));

    auto versionString = packageInfo->GetVersion();
    RETURN_IF_FAILED(packageKey.SetStringValue(L"DisplayVersion", versionString));

    std::wstring packageIconString = m_msixRequest->GetPackageDirectoryPath() + L"\\" + packageInfo->GetRelativeExecutableFilePath();
    RETURN_IF_FAILED(packageKey.SetStringValue(L"DisplayIcon", packageIconString));

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Added Uninstall key successfully",
        TraceLoggingValue(packageFullName.c_str(), "packageFullName"),
        TraceLoggingValue(uninstallCommand.c_str(), "uninstallString"),
        TraceLoggingValue(displayName.c_str(), "displayName"),
        TraceLoggingValue(directoryPath.c_str(), "installLocation"),
        TraceLoggingValue(packageInfo->GetPublisher().c_str(), "publisher"),
        TraceLoggingValue(versionString.c_str(), "displayVersion"),
        TraceLoggingValue(packageIconString.c_str(), "displayIcon"));

    return S_OK;
}

HRESULT AddRemovePrograms::ExecuteForRemoveRequest()
{
    RegistryKey uninstallKey;
    RETURN_IF_FAILED(uninstallKey.Open(HKEY_LOCAL_MACHINE, uninstallKeyPath.c_str(), KEY_WRITE));

    RETURN_IF_FAILED(uninstallKey.DeleteSubKey(m_msixRequest->GetPackageInfo()->GetPackageFullName().c_str()));

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "Removed Uninstall key successfully");
    return S_OK;
}

HRESULT AddRemovePrograms::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<AddRemovePrograms> localInstance(new AddRemovePrograms(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}
