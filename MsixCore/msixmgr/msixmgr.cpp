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
#include "CIMProvider.hpp"
#include "MsixErrors.hpp"
#include <filesystem>

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

            BOOL isUserAdmin = IsUserAnAdmin();
            TraceLoggingWrite(g_MsixUITraceLoggingProvider, "ElevationType",
                TraceLoggingValue(elevationTypeString, "ElevationTypeString"),
                TraceLoggingValue((DWORD)type, "elevationTypeEnum"),
                TraceLoggingValue(isUserAdmin, "isUserAdmin"));
            if (type == TokenElevationTypeFull || (type == TokenElevationTypeDefault && isUserAdmin))
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

            auto packageSourcePath = cli.GetPackageFilePathToInstall();
            auto unpackDestination = cli.GetUnpackDestination();
            auto rootDirectory = cli.GetRootDirectory();
            WVDFileType fileType = cli.GetFileType();
            bool createFile = cli.IsCreate();

            if (fileType == WVDFileType::CIM)
            {
                if (rootDirectory.empty() || fileType == WVDFileType::NotSpecified)
                {
                    std::wcout << std::endl;
                    std::wcout << "Creating a file with the -create option requires both a -rootDirectory and -fileType" << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }
                if (cli.IsApplyACLs())
                {
                    std::wcout << std::endl;
                    std::wcout << "Applying ACLs is not applicable for CIM files." << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }
                if (!EndsWith(unpackDestination, L".cim"))
                {
                    std::wcout << std::endl;
                    std::wcout << "Invalid CIM file name." << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }

                // Create a temporary directory to unpack package(s) since we cannot unpack to the CIM directly.
                std::wstring currentDirectory = std::filesystem::current_path();
                GUID uniqueId;
                RETURN_IF_FAILED(createGUID(&uniqueId));
                RPC_STATUS status = UuidCreate(&uniqueId);
                if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
                {
                    return HRESULT_FROM_WIN32(status);
                }
                RPC_WSTR uniqueIDRPCString = NULL;
                std::wstring uniqueIDString;
                if (UuidToStringW(&uniqueId, &uniqueIDRPCString) == RPC_S_OK)
                {
                    uniqueIDString = (WCHAR*) uniqueIDRPCString;
                    RpcStringFreeW(&uniqueIDRPCString);
                }
                std::wstring tempDirPathString = currentDirectory + L"\\" + uniqueIDString;
                std::filesystem::path tempDirPath(tempDirPathString);
                bool createTempDirResult = std::filesystem::create_directory(tempDirPath);
                // TO-DO: Proper error handling
                if (!createTempDirResult)
                {
                    std::wcout << std::endl;
                    std::wcout << "Failed to create temp directory" << std::endl;
                    std::wcout << std::endl;
                    return E_FAIL;
                }
                  
                RETURN_IF_FAILED(MsixCoreLib::Unpack(packageSourcePath, tempDirPathString, false /*applyACLs*/, cli.IsValidateSignature()));

                RETURN_IF_FAILED(MsixCoreLib::CreateAndAddToCIM(unpackDestination, tempDirPathString, rootDirectory));

                // Best-effort attempt to remove temp directory
                std::filesystem::remove_all(tempDirPath);
            }
            // UnpackDestinationFileType::NotSpecified is only valid if unpacking to an existing VHD
            else if (fileType == WVDFileType::NotSpecified || fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                if (createFile)
                {
                    // TO-DO: Add ability to create VHD

                    //ULONGLONG size = 500 * 1024 * 1024;
                    //std::wstring mountPoint;
                    //hr = MsixCoreLib::CreateAndMount(size, packageFilePath.c_str(), mountPoint);
                    //if (FAILED(hr))
                    //{
                    //    std::wcout << std::endl;
                    //    std::wcout << "Failed to create and mount vhd with error: " << hr << std::endl;
                    //    std::wcout << std::endl;
                    //    return hr;
                    //}
                    //else
                    //{
                    //    std::wcout << std::endl;
                    //    std::wcout << "Successfully created VHD and mounted to drive: " << mountPoint.c_str() << std::endl;
                    //    std::wcout << std::endl;
                    //}

                    return ERROR_NOT_SUPPORTED;
                }
                else
                {
                    RETURN_IF_FAILED(MsixCoreLib::Unpack(packageSourcePath, unpackDestination, cli.IsApplyACLs(), cli.IsValidateSignature()));
                }
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
        case OperationType::MountImage:
        {
            if (cli.GetFileType() == WVDFileType::CIM)
            {
                if (cli.GetVolumeId().empty())
                {
                    std::wcout << std::endl;
                    std::wcout << "Please provide a volume id in order to mount a CIM image" << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }

                std::wstring volumeIdString = cli.GetVolumeId();
                GUID volumeIdFromString;
                if (UuidFromStringW((RPC_WSTR)(cli.GetVolumeId().c_str()), &volumeIdFromString) != RPC_S_OK)
                {
                    std::wcout << std::endl;
                    std::wcout << "Failed to convert specified volume id {" << volumeIdString << "}  to GUID" << std::endl;
                    std::wcout << std::endl;
                    return E_UNEXPECTED;
                }

                RETURN_IF_FAILED(MsixCoreLib::MountCIM(cli.GetMountImagePath(), volumeIdFromString));

                std::wcout << std::endl;
                std::wcout << "Image successfully mounted!" << std::endl;
                std::wcout << "To examine contents in File Explorer, press Win + R and enter the following: " << std::endl;
                std::wcout << "\\\\?\\Volume{" << volumeIdString << "}" << std::endl;
                std::wcout << std::endl;
                std::wcout << "To unmount, run the following command: " << std::endl;
                std::wcout << "msixmgr.exe -unmountimage -volumeid " << volumeIdString << " -filetype CIM" << std::endl;
                std::wcout << std::endl;
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -MountImage command: {CIM}" << std::endl;
                std::wcout << std::endl;
                return ERROR_NOT_SUPPORTED;
            }
            return S_OK;
        }
        case OperationType::UnmountImage:
        {
            if (cli.GetFileType() == WVDFileType::CIM)
            {
                if (cli.GetVolumeId().empty())
                {
                    std::wcout << std::endl;
                    std::wcout << "Please provide the id of the volume you would like to unmount using the -volumeId option" << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }

                std::wstring volumeIdString = cli.GetVolumeId();
                GUID volumeIdFromString;
                if (UuidFromStringW((RPC_WSTR)(cli.GetVolumeId().c_str()), &volumeIdFromString) != RPC_S_OK)
                {
                    std::wcout << std::endl;
                    std::wcout << "Failed to convert specified volume id {" << volumeIdString << "}  to GUID" << std::endl;
                    std::wcout << std::endl;
                    return E_UNEXPECTED;
                }

                RETURN_IF_FAILED(MsixCoreLib::UnmountCIM(volumeIdFromString));
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -UnmountImage command: {CIM}" << std::endl;
                std::wcout << std::endl;
                return ERROR_NOT_SUPPORTED;
            }
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
