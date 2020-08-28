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
#include "VHDProvider.hpp"
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

void OutputUnpackFailures(
    _In_ std::wstring packageSource,
    _In_ std::vector<std::wstring> skippedFiles,
    _In_ std::vector<std::wstring> failedPackages,
    _In_ std::vector<HRESULT> failedPackagesErrors)
{
    if (!skippedFiles.empty())
    {
        std::wcout << std::endl;
        std::wcout << "[WARNING] The following items from " << packageSource << " were ignored because they are not packages or bundles " << std::endl;
        std::wcout << std::endl;

        for (int i = 0; i < skippedFiles.size(); i++)
        {
            std::wcout << skippedFiles.at(i) << std::endl;
        }

        std::wcout << std::endl;
    }

    if (!failedPackages.empty())
    {
        std::wcout << std::endl;
        std::wcout << "[WARNING] The following packages from " << packageSource << " failed to get unpacked. Please try again: " << std::endl;
        std::wcout << std::endl;

        for (int i = 0; i < failedPackages.size(); i++)
        {
            HRESULT hr = failedPackagesErrors.at(i);

            std::wcout << L"Failed with HRESULT 0x" << std::hex << hr << L" when trying to unpack " << failedPackages.at(i) << std::endl;
            if (hr == static_cast<HRESULT>(MSIX::Error::CertNotTrusted))
            {
                std::wcout << L"Please confirm that the certificate has been installed for this package" << std::endl;
            }
            else if (hr == static_cast<HRESULT>(MSIX::Error::FileWrite))
            {
                std::wcout << L"The tool encountered a file write error. If you are unpacking to a VHD, please try again with a larger VHD, as file write errors may be caused by insufficient disk space." << std::endl;
            }
            else if (hr == E_INVALIDARG)
            {
                std::wcout << "Please confirm the given package path is an .appx, .appxbundle, .msix, or .msixbundle file" << std::endl;
            }

            std::wcout << std::endl;
        }
    }
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
                if (!EndsWith(unpackDestination, L".cim"))
                {
                    std::wcout << std::endl;
                    std::wcout << "Invalid CIM file name. File name must have .cim file extension" << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }

                // Create a temporary directory to unpack package(s) since we cannot unpack to the CIM directly.
                std::wstring currentDirectory = std::filesystem::current_path();
                std::wstring uniqueIdString;
                RETURN_IF_FAILED(CreateGUIDString(&uniqueIdString));
                std::wstring tempDirPathString = currentDirectory + L"\\" + uniqueIdString;
                std::filesystem::path tempDirPath(tempDirPathString);

                std::error_code createDirectoryErrorCode;
                bool createTempDirResult = std::filesystem::create_directory(tempDirPath, createDirectoryErrorCode);

                // Since we're using a GUID, this should almost never happen
                if (!createTempDirResult)
                {
                    std::wcout << std::endl;
                    std::wcout << "Failed to create temp directory " << tempDirPathString << std::endl;
                    std::wcout << "This may occur when the directory path already exists. Please try again."  << std::endl;
                    std::wcout << std::endl;
                    return E_UNEXPECTED;
                }
                if (createDirectoryErrorCode.value() != 0)
                {
                    // Again, we expect that the creation of the temp directory will fail very rarely. Output the exception
                    // and have the user try again.
                    std::wcout << std::endl;
                    std::wcout << "Creation of temp directory " << tempDirPathString << " failed with error: " << createDirectoryErrorCode.value() << std::endl;
                    std::cout << "Error message: " << createDirectoryErrorCode.message() << std::endl;
                    std::wcout << "Please try again." << std::endl;
                    std::wcout << std::endl;
                    return E_UNEXPECTED;
                }

                std::vector<std::wstring> skippedFiles;
                std::vector<std::wstring> failedPackages;
                std::vector<HRESULT> failedPackagesErrors;
                RETURN_IF_FAILED(MsixCoreLib::Unpack(
                    packageSourcePath,
                    tempDirPathString,
                    cli.IsApplyACLs(),
                    cli.IsValidateSignature(),
                    skippedFiles,
                    failedPackages,
                    failedPackagesErrors));

                HRESULT hrCreateCIM = MsixCoreLib::CreateAndAddToCIM(unpackDestination, tempDirPathString, rootDirectory);

                // Best-effort attempt to remove temp directory
                std::error_code removeTempDirErrorCode;
                bool removeTemprDirResult = std::filesystem::remove_all(tempDirPath, removeTempDirErrorCode);
                if (!removeTemprDirResult || removeTempDirErrorCode.value() != 0)
                {
                    std::wcout << std::endl;
                    std::wcout << "Failed to remove the temp dir  " << tempDirPath << std::endl;
                    std::wcout << "Ignoring this non-fatal error and moving on" << std::endl;
                    std::wcout << std::endl;
                }

                if (FAILED(hrCreateCIM))
                {
                    std::wcout << std::endl;
                    std::wcout << "Creating the CIM file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hr << std::endl;
                    std::wcout << std::endl;
                    return hrCreateCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully created the CIM file: " << unpackDestination << std::endl;
                    std::wcout << std::endl;

                    OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors);
                }
                 
            }
            // UnpackDestinationFileType::NotSpecified is only valid if unpacking to an existing VHD
            else if (fileType == WVDFileType::NotSpecified || fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                if (createFile)
                {
                    if (!(EndsWith(unpackDestination, L".vhd") || (EndsWith(unpackDestination, L".vhdx"))))
                    {
                        std::wcout << std::endl;
                        std::wcout << "Invalid VHD file name. File name must have .vhd or .vhdx file extension" << std::endl;
                        std::wcout << std::endl;
                        return E_INVALIDARG;
                    }
                    if (EndsWith(packageSourcePath, L"detach"))
                    {
                        HRESULT hrMountVHD = MsixCoreLib::UnmountVHD(unpackDestination);
                        if (FAILED(hrMountVHD))
                        {
                            std::wcout << std::endl;
                            std::wcout << "Unmounting the VHD file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrMountVHD << std::endl;
                            std::wcout << std::endl;
                            return hrMountVHD;
                        }
                        return S_OK;
                    }

                    if (fileType == WVDFileType::VHDX)
                    {
                        WCHAR volumeName[MAX_PATH];
                        DWORD bytesReturned;
                        VOLUME_DISK_EXTENTS diskExtents;
                        HANDLE hFVol = FindFirstVolumeW(volumeName, sizeof(volumeName));

                        do
                        {
                            std::wcout << std::endl;
                            std::wcout << "Volume Name: " << volumeName << std::endl;

                            //size_t backslashPos = wcslen(volumeName) - 1;
                            //bool hadTrailingBackslash = volumeName[backslashPos] == '\\';
                            //if (hadTrailingBackslash) {
                            //    volumeName[backslashPos] = 0;
                            //}

                            //HANDLE hVol = CreateFile(volumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                            //if (hVol == INVALID_HANDLE_VALUE) {
                            //    return E_FAIL;
                            //}

                            //STORAGE_DEVICE_NUMBER_EX volStorageDeviceInfo = {};
                            //DWORD volBytesReturned;
                            //BOOL res = DeviceIoControl(hVol, IOCTL_STORAGE_GET_DEVICE_NUMBER_EX, nullptr,
                            //    0, &volStorageDeviceInfo, sizeof(volStorageDeviceInfo), &volBytesReturned, nullptr);
                            //if (!res)
                            //{
                            //    DWORD error = GetLastError();
                            //    std::wcout << std::endl;
                            //    std::wcout << "Failed to query for vol storage device info: " << error  << std::endl;
                            //    std::wcout << std::endl;
                            //}

                            //DWORD  volDeviceNumber = volStorageDeviceInfo.DeviceNumber;
                            //std::wcout << "volDeviceNumber: " << volDeviceNumber << std::endl;

                            std::wcout << std::endl;

                            // I had a problem where CreateFile complained about the trailing \ and
                            // SetVolumeMountPoint desperately wanted the backslash there. I ended up 
                            // doing this to get it working but I'm not a fan and I'd greatly 
                            // appreciate it if someone has any further info on this matter
                            int backslashPos = wcslen(volumeName) - 1;
                            bool hadTrailingBackslash = volumeName[backslashPos] == '\\';
                            if (hadTrailingBackslash) {
                                volumeName[backslashPos] = 0;
                            }

                            HANDLE hVol = CreateFile(volumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                            if (hVol == INVALID_HANDLE_VALUE) {
                                return E_FAIL;
                            }

                            DeviceIoControl(hVol, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL,
                                0, &diskExtents, sizeof(diskExtents), &bytesReturned, NULL);

                            for (int i = 0; i < diskExtents.NumberOfDiskExtents; i++)
                            {
                                std::wcout << "Disk Extent : " << i<< " has disk number" << diskExtents.Extents[i].DiskNumber << std::endl;
                            }


                            //// If the volume were to span across multiple physical disks, you'd find 
                            //// more than one Extents here but we don't have to worry about that with VHD
                            //// Note that 'driveNumber' would be the integer you extracted out of 
                            //// 'physicalDrive' in the previous snippet
                            //if (diskExtents.Extents[0].DiskNumber == driveNumber) {
                            //    if (hadTrailingBackslash) {
                            //        volumeName[backslashPos] = '\\';
                            //    }

                            //    // Found volume that's on the VHD, let's mount it with a letter of our choosing.
                            //    // Warning: SetVolumeMountPoint requires elevation
                            //    SetVolumeMountPoint("H:\\", volumeName);
                            
                        } while (FindNextVolume(hFVol, volumeName, sizeof(volumeName)));
                        FindVolumeClose(hFVol);
                    }
                    else
                    {
                        HRESULT hrCreateVHD = MsixCoreLib::CreateAndMountVHD(unpackDestination);
                        if (FAILED(hrCreateVHD))
                        {
                            std::wcout << std::endl;
                            std::wcout << "Creating the VHD file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateVHD << std::endl;
                            std::wcout << std::endl;
                            return hrCreateVHD;
                        }
                    }
                }
                else
                {
                    std::vector<std::wstring> skippedFiles;
                    std::vector<std::wstring> failedPackages;
                    std::vector<HRESULT> failedPackagesErrors;
                    RETURN_IF_FAILED(MsixCoreLib::Unpack(
                        packageSourcePath,
                        unpackDestination,
                        cli.IsApplyACLs(),
                        cli.IsValidateSignature(),
                        skippedFiles,
                        failedPackages,
                        failedPackagesErrors));

                    std::wcout << std::endl;
                    std::wcout << "Finished unpacking packages to: " << unpackDestination << std::endl;
                    std::wcout << std::endl;

                    OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors);
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
            WVDFileType fileType = cli.GetFileType();
            if (fileType == WVDFileType::CIM)
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
                    std::wcout << "Failed to convert specified volume id {" << volumeIdString << "} to GUID" << std::endl;
                    std::wcout << std::endl;
                    return E_UNEXPECTED;
                }

                
                HRESULT hrMountCIM = MsixCoreLib::MountCIM(cli.GetMountImagePath(), volumeIdFromString);
                if (FAILED(hrMountCIM))
                {
                    std::wcout << std::endl;
                    std::wcout << "Mounting the CIM file  " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrMountCIM << std::endl;
                    std::wcout << std::endl;
                    return hrMountCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Image successfully mounted!" << std::endl;
                    std::wcout << "To examine contents in File Explorer, press Win + R and enter the following: " << std::endl;
                    std::wcout << "\\\\?\\Volume{" << volumeIdString << "}" << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "To unmount, run the following command: " << std::endl;
                    std::wcout << "msixmgr.exe -unmountimage -volumeid " << volumeIdString << " -filetype CIM" << std::endl;
                    std::wcout << std::endl;
                }
            }
            else if (fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                if (cli.GetMountImagePath().empty())
                {
                    std::wcout << std::endl;
                    std::wcout << "Please provide the path to the image you would like to mount." << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }

                HRESULT hrMountVHD = MsixCoreLib::MountVHD(cli.GetMountImagePath());
                if (FAILED(hrMountVHD))
                {
                    std::wcout << std::endl;
                    std::wcout << "Mounting the VHD(X) file  " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrMountVHD << std::endl;
                    std::wcout << std::endl;
                    return hrMountVHD;
                }
                else
                {
                    //std::wcout << std::endl;
                    std::wcout << "Image successfully mounted!" << std::endl;
                    //std::wcout << "To examine contents in File Explorer, press Win + R and enter the following: " << std::endl;
                    //std::wcout << "\\\\?\\Volume{" << volumeIdString << "}" << std::endl;
                    //std::wcout << std::endl;
                    //std::wcout << "To unmount, run the following command: " << std::endl;
                    //std::wcout << "msixmgr.exe -unmountimage -volumeid " << volumeIdString << " -filetype CIM" << std::endl;
                    //std::wcout << std::endl;
                }

            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -MountImage command: {VHD, VHDX, CIM}" << std::endl;
                std::wcout << std::endl;
                return ERROR_NOT_SUPPORTED;
            }
            return S_OK;
        }
        case OperationType::UnmountImage:
        {
            WVDFileType fileType = cli.GetFileType();
            if (fileType == WVDFileType::CIM)
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

                HRESULT hrUnmountCIM = MsixCoreLib::UnmountCIM(volumeIdFromString);

                if (FAILED(hrUnmountCIM))
                {
                    std::wcout << std::endl;
                    std::wcout << "Unmounting the CIM with volume id  " << volumeIdString << " failed with HRESULT 0x" << std::hex << hrUnmountCIM << std::endl;
                    std::wcout << std::endl;
                    return hrUnmountCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully unmounted the CIM with volume id " << volumeIdString << std::endl;
                    std::wcout << std::endl;
                }
            }
            else if (fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                if (cli.GetMountImagePath().empty())
                {
                    std::wcout << std::endl;
                    std::wcout << "Please provide the path to the image you would like to unmount." << std::endl;
                    std::wcout << std::endl;
                    return E_INVALIDARG;
                }

                HRESULT hrUnmountVHD = MsixCoreLib::UnmountVHD(cli.GetMountImagePath());

                if (FAILED(hrUnmountVHD))
                {
                    std::wcout << std::endl;
                    std::wcout << "Unmounting the VHD " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrUnmountVHD << std::endl;
                    std::wcout << std::endl;
                    return hrUnmountVHD;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully unmounted the VHD " << cli.GetMountImagePath() << std::endl;
                    std::wcout << std::endl;
                }
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -UnmountImage command: {VHD, VHDX, CIM}" << std::endl;
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
