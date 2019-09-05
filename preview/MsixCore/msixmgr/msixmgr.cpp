// msixmgr.cpp :
// The main entry point for msixmgr.exe. This application manages various facets of msix packages, including
// a working preview for the MSIX/APPX installer for Windows 7 SP1 and higher OS versions
#include "MSIXWindows.hpp"
#include <shlobj_core.h>
#include <CommCtrl.h>

#include <string>
#include <iostream>
#include <vector>
#include <TraceLoggingProvider.h>
#include "InstallUI.hpp"
#include "CommandLineInterface.hpp"
#include "msixmgrLogger.hpp"
#include "Util.hpp"
#include "..\msixmgrLib\GeneralUtil.hpp"
#include "resource.h"
#include <VersionHelpers.h>
#include "UnpackProvider.hpp"
#include "ApplyACLsProvider.hpp"
#include "MsixErrors.hpp"

#include <msixmgrActions.hpp>
using namespace std;
using namespace MsixCoreLib;

TRACELOGGING_DECLARE_PROVIDER(g_MsixTraceLoggingProvider);

int main(int argc, char * argv[])
{
    // Register the providers
    TraceLoggingRegister(g_MsixUITraceLoggingProvider);
    TraceLoggingRegister(g_MsixTraceLoggingProvider);

    HRESULT hrCoInitialize = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hrCoInitialize))
    {
        std::wcout << GetStringResource(IDS_STRING_FAILED_COM_INITIALIZATION) << " " << std::hex << hrCoInitialize << std::endl;
        return 1;
    }

    CommandLineInterface cli(argc, argv);

    const HRESULT hrCreateRequest = cli.Init();
    if (SUCCEEDED(hrCreateRequest))
    {
        switch (cli.GetOperationType())
        {
        case OperationType::Add:
        {
            AutoPtr<IPackageManager> packageManager;
            RETURN_IF_FAILED(MsixCoreLib_CreatePackageManager(&packageManager));

            if (cli.IsQuietMode())
            {
                HRESULT hrAddPackage = packageManager->AddPackage(cli.GetPackageFilePathToInstall(), DeploymentOptions::None);
                if (FAILED(hrAddPackage))
                {
                    std::wcout << GetStringResource(IDS_STRING_FAILED_REQUEST) << " " << std::hex << hrAddPackage << std::endl;
                    return hrAddPackage;
                }
            }
            else
            {
                if (IsWindows10RS3OrLater())
                {
                    const int bufSize = 1024;
                    wchar_t path[bufSize];
                    if (!GetFullPathNameW(cli.GetPackageFilePathToInstall().c_str(), bufSize, path, nullptr))
                    {
                        return HRESULT_FROM_WIN32(GetLastError());
                    }

                    std::wstring protocol = std::wstring(L"ms-appinstaller:?source=");
                    protocol.append(path);

                    ShellExecuteW(nullptr, L"Open", protocol.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                }
                else
                {
                    auto ui = new UI(packageManager, cli.GetPackageFilePathToInstall(), UIType::InstallUIAdd);
                    ui->ShowUI();
                }
            }
            break;
        }
        case OperationType::Remove:
        {
            AutoPtr<IPackageManager> packageManager;
            RETURN_IF_FAILED(MsixCoreLib_CreatePackageManager(&packageManager));

            auto packageFullName = cli.GetPackageFullName();
            HRESULT hrRemovePackage = packageManager->RemovePackage(packageFullName);
            if (FAILED(hrRemovePackage))
            {
                std::wcout << GetStringResource(IDS_STRING_FAILED_REQUEST) << " " << std::hex << hrRemovePackage << std::endl;
                return hrRemovePackage;
            }
            break;
        }
        case OperationType::FindPackage:
        {
            AutoPtr<IPackageManager> packageManager;
            RETURN_IF_FAILED(MsixCoreLib_CreatePackageManager(&packageManager));

            shared_ptr<IInstalledPackage> packageInfo;
            HRESULT hr = packageManager->FindPackage(cli.GetPackageFullName(), packageInfo);

            if (packageInfo == NULL || hr == ERROR_NOT_FOUND)
            {
                hr = packageManager->FindPackageByFamilyName(cli.GetPackageFullName(), packageInfo);
                if (FAILED(hr))
                {
                    std::wcout << L"Failed to determine findpackage results" << hr << std::endl;
                }
                else if (packageInfo == NULL || hr == ERROR_NOT_FOUND)
                {
                    std::wcout << std::endl;
                    std::wcout << L"No packages found " << HRESULT_FROM_WIN32(hr) << std::endl;
                    std::wcout << std::endl;
                }
            }

            if (packageInfo != nullptr)
            {
                std::wcout << std::endl;
                std::wcout << L"PackageFullName: " << packageInfo->GetPackageFullName().c_str() << std::endl;
                std::wcout << L"DisplayName: " << packageInfo->GetDisplayName().c_str() << std::endl;

                std::wcout << L"DirectoryPath: " << packageInfo->GetInstalledLocation().c_str() << std::endl;
                std::wcout << std::endl;

            }

            return S_OK;
        }
        case OperationType::FindAllPackages:
        {
            AutoPtr<IPackageManager> packageManager;
            RETURN_IF_FAILED(MsixCoreLib_CreatePackageManager(&packageManager));

            std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> packages;
            RETURN_IF_FAILED(packageManager->FindPackages(packages));

            unsigned int numPackages = 0;
            for (auto& package : *packages)
            {
                std::wcout << package->GetPackageFullName() << std::endl;
                numPackages++;
            }

            std::cout << numPackages << " Package(s) found" << std::endl;
            return S_OK;
        }
        case OperationType::Unpack:
        {
            HRESULT hr = S_OK;

            auto packageFilePath = cli.GetPackageFilePathToInstall();
            auto unpackDestination = cli.GetUnpackDestination();

            if (IsPackageFile(packageFilePath))
            {
                hr = MsixCoreLib::UnpackPackage(packageFilePath, unpackDestination, cli.IsApplyACLs());
            }
            else if (IsBundleFile(packageFilePath))
            {
                hr = MsixCoreLib::UnpackBundle(packageFilePath, unpackDestination, cli.IsApplyACLs());
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Invalid package path: " << packageFilePath << std::endl;
                std::wcout << "Please confirm the given package path is an .appx, .appxbundle, .msix, or .msixbundle file" << std::endl;
                std::wcout << std::endl;
                return E_INVALIDARG;
            }
            if (FAILED(hr))
            {
                std::wcout << std::endl;
                std::wcout << L"Failed with HRESULT 0x" << std::hex << hr << L" when trying to unpack " << packageFilePath << std::endl;
                if (hr == static_cast<HRESULT>(MSIX::Error::CertNotTrusted))
                {
                    std::wcout << L"Please confirm that the certificate has been installed for this package" << std::endl;
                }
                std::wcout << std::endl;
            }

            return hr;
        }
        case OperationType::ApplyACLs:
        {
            std::vector<std::wstring> packageFolders;
            packageFolders.push_back(cli.GetPackageFilePathToInstall()); // we're not actually installing anything. The API just returns the file path name we need.
            RETURN_IF_FAILED(MsixCoreLib::ApplyACLs(packageFolders));
            return S_OK;
        }
        default:
            return E_NOT_SET;
        }
    }
    else
    {
        cli.DisplayHelp();
    }

    // Stop TraceLogging and unregister the providers
    TraceLoggingUnregister(g_MsixUITraceLoggingProvider);
    TraceLoggingUnregister(g_MsixTraceLoggingProvider);

    return 0;
}
