// MsixCoreInstaller.cpp :
// The main entry point for MsixCoreInstaller.exe. This application is
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
#include "MsixCoreInstallerLogger.hpp"
#include "Util.hpp"
#include "..\MsixCoreInstallerLib\GeneralUtil.hpp"
#include "resource.h"
#include <VersionHelpers.h>
#include <MsixCoreInstallerActions.hpp>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Management.Deployment.h>

using namespace std;
using namespace MsixCoreLib;

BOOL IsWindows10RS3OrLater()
{
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 16299;

    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

    return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask);
}


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
            // Before doing any actual processing of an add request, see if this is RS3 or later.
            if (IsWindows10RS3OrLater())
            {
             //*
                std::wstring packageUriString{ cli.GetPackageFilePathToInstall() };
                winrt::Windows::Foundation::Uri packageUri{ packageUriString };
                winrt::Windows::Management::Deployment::PackageManager packageManager;

                auto deploymentOperation{ packageManager.AddPackageAsync(packageUri, nullptr, winrt::Windows::Management::Deployment::DeploymentOptions::None) };
                deploymentOperation.get();
              // */ 
                // use the desktopappinstaller UI
                //std::wstring protocol = std::wstring(L"ms-appinstaller:?source=") + m_packageFilePath;
                //ShellExecuteW(nullptr, L"Open", protocol.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

                exit(0);
            }

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

                auto ui = new UI(packageManager, cli.GetPackageFilePathToInstall(), UIType::InstallUIAdd);
                ui->ShowUI();
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
            if (packageInfo == NULL || FAILED(hr))
            {
                std::wcout << std::endl;
                std::wcout << L"No packages found " << hr << std::endl;
                std::wcout << std::endl;
            }
            else
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
