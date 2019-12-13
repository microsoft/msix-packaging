// msixmgr.cpp :
// The main entry point for msixmgr.exe. This application manages various facets of msix packages, including
// a working preview for the MSIX/APPX installer for Windows 7 SP1 and higher OS versions
#include "MSIXWindows.hpp"
#include <shlobj_core.h>
#include <CommCtrl.h>

#include <string>
#include <sstream>
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

HRESULT LogFileInfo()
{
    WCHAR filePath[MAX_PATH];
    DWORD lengthCopied = GetModuleFileNameW(nullptr, filePath, MAX_PATH);
    if (lengthCopied == 0)
    {
        RETURN_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }

    UINT64 version = 0;
    bool isUnversioned = false;
    RETURN_IF_FAILED(GetFileVersion(filePath, version, isUnversioned));

    std::wstring versionString =
        std::to_wstring((version & 0xFFFF000000000000) >> 48) + L"." +
        std::to_wstring((version & 0x0000FFFF00000000) >> 32) + L"." +
        std::to_wstring((version & 0x00000000FFFF0000) >> 16) + L"." +
        std::to_wstring(version & 0x000000000000FFFF);
    TraceLoggingWrite(g_MsixUITraceLoggingProvider,
        "msixmgr.exe file version",
        TraceLoggingValue(versionString.c_str(), "Version"));
    return S_OK;
}

bool IsAdmin()
{
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
    {
        TOKEN_ELEVATION_TYPE type;
        DWORD length = sizeof(TOKEN_ELEVATION_TYPE);
        if (GetTokenInformation(token, TokenElevationType, &type, sizeof(TOKEN_ELEVATION_TYPE), &length))
        {
            PWSTR elevationTypeString = nullptr;
            switch (type)
            {
            case TokenElevationTypeDefault:
                elevationTypeString = L"DefaultType";
                break;
            case TokenElevationTypeFull:
                elevationTypeString = L"Full";
                break;
            case TokenElevationTypeLimited:
                elevationTypeString = L"Limited";
                break;
            default:
                elevationTypeString = L"unknown";
                break;
            }

            TraceLoggingWrite(g_MsixUITraceLoggingProvider, "ElevationType",
                TraceLoggingValue(elevationTypeString, "ElevationTypeString"),
                TraceLoggingValue((DWORD)type, "elevationTypeEnum"));
            if (type == TokenElevationTypeFull)
            {
                return true;
            }
        }
    }

    return false;
}

void RelaunchAsAdmin(int argc, char * argv[])
{
    // free the console window so it's not showing when UAC prompt shows.
    FreeConsole();

    SHELLEXECUTEINFOW shellExecuteInfo = {};
    shellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    shellExecuteInfo.fMask = SEE_MASK_DEFAULT;
    shellExecuteInfo.lpVerb = L"runas";
    shellExecuteInfo.lpFile = L"msixmgr.exe";
    shellExecuteInfo.nShow = SW_SHOW;

    std::wstringstream argumentstream;
    for (int i = 1; i < argc; i++)
    {
        argumentstream << argv[i] << L" ";
    }
    std::wstring args = argumentstream.str();

    shellExecuteInfo.lpParameters = args.c_str();

    TraceLoggingWrite(g_MsixUITraceLoggingProvider, "Relaunching as admin",
        TraceLoggingValue(args.c_str(), "args"));

    ShellExecuteExW(&shellExecuteInfo);
}

int main(int argc, char * argv[])
{
    // Register the providers
    TraceLoggingRegister(g_MsixUITraceLoggingProvider);
    TraceLoggingRegister(g_MsixTraceLoggingProvider);

    // Determine if running as admin up front to log the result in all codepaths
    bool isAdmin = IsAdmin();

    HRESULT hrCoInitialize = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hrCoInitialize))
    {
        std::wcout << GetStringResource(IDS_STRING_FAILED_COM_INITIALIZATION) << " " << std::hex << hrCoInitialize << std::endl;
        return 1;
    }

    (void)(LogFileInfo());

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
                if (!isAdmin)
                {
                    RelaunchAsAdmin(argc, argv);
                    return 0;
                }
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
                    TraceLoggingWrite(g_MsixUITraceLoggingProvider, "Windows10RS3 or later: redirecting to appInstaller");

                    std::wstring protocol = std::wstring(L"ms-appinstaller:?source=");

                    std::wstring httpPrefix(L"http");
                    bool isPathHttp = cli.GetPackageFilePathToInstall().compare(0, httpPrefix.length(), httpPrefix) == 0;
                    if (isPathHttp)
                    {
                        protocol.append(cli.GetPackageFilePathToInstall());
                    }
                    else
                    {
                        const int bufSize = 1024;
                        wchar_t path[bufSize];
                        if (!GetFullPathNameW(cli.GetPackageFilePathToInstall().c_str(), bufSize, path, nullptr))
                        {
                            return HRESULT_FROM_WIN32(GetLastError());
                        }

                        protocol.append(path);
                    }
                    ShellExecuteW(nullptr, L"Open", protocol.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                }
                else
                {
                    if (!isAdmin)
                    {
                        RelaunchAsAdmin(argc, argv);
                        return 0;
                    }
                    auto ui = new UI(packageManager, cli.GetPackageFilePathToInstall(), UIType::InstallUIAdd);
                    ui->ShowUI();
                }
            }
            break;
        }
        case OperationType::Remove:
        {
            if (!isAdmin)
            {
                RelaunchAsAdmin(argc, argv);
                return 0;
            }
            FreeConsole();
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

            std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> packages;
            RETURN_IF_FAILED(packageManager->FindPackages(cli.GetPackageFullName(), packages));

            if (packages != nullptr)
            {
                unsigned int numPackages = 0;
                for (auto& package : *packages)
                {
                    std::wcout << std::endl;
                    std::wcout << L"PackageFullName: " << package->GetPackageFullName().c_str() << std::endl;
                    std::wcout << L"DisplayName: " << package->GetDisplayName().c_str() << std::endl;

                    std::wcout << L"DirectoryPath: " << package->GetInstalledLocation().c_str() << std::endl;
                    std::wcout << std::endl;
                    numPackages++;
                }

                std::cout << numPackages << " Package(s) found" << std::endl;
            }      

            return S_OK;
        }
        case OperationType::Unpack:
        {
            HRESULT hr = S_OK;

            auto packageFilePath = cli.GetPackageFilePathToInstall();
            auto unpackDestination = cli.GetUnpackDestination();

            if (IsPackageFile(packageFilePath))
            {
                hr = MsixCoreLib::UnpackPackage(packageFilePath, unpackDestination, cli.IsApplyACLs(), cli.IsValidateSignature());
            }
            else if (IsBundleFile(packageFilePath))
            {
                hr = MsixCoreLib::UnpackBundle(packageFilePath, unpackDestination, cli.IsApplyACLs(), cli.IsValidateSignature());
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
                else if (hr == static_cast<HRESULT>(MSIX::Error::FileWrite))
                {
                    std::wcout << L"The tool encountered a file write error. If you are unpacking to a VHD, please try again with a larger VHD, as file write errors may be caused by insufficient disk space." << std::endl;
                }
                std::wcout << std::endl;
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Successfully unpacked and applied ACLs for package: " << packageFilePath << std::endl;
                std::wcout << "If your package is a store-signed package, please note that store-signed apps require a license file to be included, which can be downloaded from the Microsoft Store for Business"  << std::endl;
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
