// Win7MSIXInstaller.cpp :
// The main entry point for Win7MSIXInstaller.exe. This application is
// a working preview for the MSIX/APPX installer for Windows 7
#include "MSIXWindows.hpp"
#include <shlobj_core.h>
#include <CommCtrl.h>

#include <string>
#include <iostream>
#include <vector>
#include <TraceLoggingProvider.h>
#include "InstallUI.hpp"
#include "CommandLineInterface.hpp"
#include "Win7MSIXInstallerLogger.hpp"
#include "Util.hpp"
#include "resource.h"
#include <VersionHelpers.h>

#include <Win7MSIXInstallerActions.hpp>
using namespace std;

int main(int argc, char * argv[])
{
    // Register the provider
    TraceLoggingRegister(g_MsixUITraceLoggingProvider);

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
            AutoPtr<Win7MsixInstallerLib::IPackageManager> packageManager;
            RETURN_IF_FAILED(Win7MsixInstallerLib_CreatePackageManager(&packageManager));

            if (cli.IsQuietMode())
            {
                auto res = packageManager->AddPackage(cli.GetPackageFilePathToInstall(), DeploymentOptions::None);
                if (FAILED(res))
                {
                    return res;
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
            AutoPtr<Win7MsixInstallerLib::IPackageManager> packageManager;
            RETURN_IF_FAILED(Win7MsixInstallerLib_CreatePackageManager(&packageManager));

            auto packageFullName = cli.GetPackageFullName();
            auto res = packageManager->RemovePackage(packageFullName);
            if (FAILED(res))
            {
                return res;
            }
            break;
        }
        case OperationType::FindPackage:
        {
            AutoPtr<Win7MsixInstallerLib::IPackageManager> packageManager;
            RETURN_IF_FAILED(Win7MsixInstallerLib_CreatePackageManager(&packageManager));

            shared_ptr<Win7MsixInstallerLib::IInstalledPackageInfo> packageInfo = packageManager->FindPackage(cli.GetPackageFullName());
            if (packageInfo == NULL)
            {
                std::wcout << std::endl;
                std::wcout << L"No packages found" << std::endl;
                std::wcout << std::endl;
            }
            else {
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
            AutoPtr<Win7MsixInstallerLib::IPackageManager> packageManager;
            RETURN_IF_FAILED(Win7MsixInstallerLib_CreatePackageManager(&packageManager));

            auto packages = packageManager->FindPackages();

            unsigned int numPackages = 0;
            for (auto& package : *packages)
            {
                std::wcout << package->GetPackageFullName() << std::endl;
                numPackages++;
            }

            std::cout << numPackages << " Packages found" << std::endl;
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


    // Stop TraceLogging and unregister the provider
    TraceLoggingUnregister(g_MsixUITraceLoggingProvider);

    return 0;
}
