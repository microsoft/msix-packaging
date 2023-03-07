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
#include "msixmgrTelemetry.hpp"
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
#include "msixmgr_tracelogging.h"

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
    TraceLoggingRegister(g_MsixMgrTelemetryProvider);

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

    // Initializing all the variables for Telemetry
    std::wstring WorkflowId, SourceApplicationId;
    double WorkflowElapsedTime;
    LARGE_INTEGER MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency;
    QueryPerformanceFrequency(&MsixMgrLoad_Frequency);

    // Creating WorkflowId for tracking the Workflows
    WorkflowId = msixmgr::CreateWorkflowId();
    SourceApplicationId = L"SELF";

    // Telemetry : Session Log
    msixmgr::TraceLogSession(WorkflowId.c_str(), SourceApplicationId.c_str());

    QueryPerformanceCounter(&MsixMgrLoad_StartCounter);

    if (SUCCEEDED(hrCreateRequest))
    {
        switch (cli.GetOperationType())
        {
        case OperationType::Add:
        {
            // Telemetry : Add Workflow Log
            msixmgr::TraceLogAddWorkflow(WorkflowId.c_str(), cli.GetPackageFilePathToInstall().c_str());

            HRESULT hr;

            AutoPtr<IPackageManager> packageManager;
            hr = MsixCoreLib_CreatePackageManager(&packageManager);

            if (FAILED(hr))
            {
                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                RETURN_IF_FAILED(hr);
            }

            if (cli.IsQuietMode())
            {
                if (!isAdmin)
                {
                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    RelaunchAsAdmin(argc, argv);
                    return 0;
                }
                HRESULT hrAddPackage = packageManager->AddPackage(cli.GetPackageFilePathToInstall(), DeploymentOptions::None);
                if (FAILED(hrAddPackage))
                {
                    std::wcout << GetStringResource(IDS_STRING_FAILED_REQUEST) << " " << std::hex << hrAddPackage << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

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
                            hr = HRESULT_FROM_WIN32(GetLastError());

                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                            return hr;
                        }

                        protocol.append(path);
                    }
                    ShellExecuteW(nullptr, L"Open", protocol.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                }
                else
                {
                    if (!isAdmin)
                    {
                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                        WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                        msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                        RelaunchAsAdmin(argc, argv);
                        return 0;
                    }
                    auto ui = new UI(packageManager, cli.GetPackageFilePathToInstall(), UIType::InstallUIAdd);
                    hr = ui->ShowUI();

                    if (FAILED(hr))
                    {
                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                        WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                        msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                        RETURN_IF_FAILED(hr);
                    }
                }
            }

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Add", true, WorkflowElapsedTime, L"", L"");

            break;
        }
        case OperationType::Remove:
        {
            // Telemetry : Remove Workflow Log
            msixmgr::TraceLogRemoveWorkflow(WorkflowId.c_str(), cli.GetPackageFullName().c_str());

            HRESULT hr;

            if (!isAdmin)
            {
                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Remove", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                RelaunchAsAdmin(argc, argv);
                return 0;
            }
            FreeConsole();
            AutoPtr<IPackageManager> packageManager;
            hr = MsixCoreLib_CreatePackageManager(&packageManager);

            if (FAILED(hr))
            {
                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Remove", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                RETURN_IF_FAILED(hr);
            }

            auto packageFullName = cli.GetPackageFullName();
            HRESULT hrRemovePackage = packageManager->RemovePackage(packageFullName);
            if (FAILED(hrRemovePackage))
            {
                std::wcout << GetStringResource(IDS_STRING_FAILED_REQUEST) << " " << std::hex << hrRemovePackage << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Remove", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                return hrRemovePackage;
            }

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Remove", true, WorkflowElapsedTime, L"", L"");

            break;
        }
        case OperationType::FindPackage:
        {
            // Telemetry : Find Workflow Log
            msixmgr::TraceLogFindWorkflow(WorkflowId.c_str(), cli.GetPackageFullName().c_str());

            HRESULT hr;
            AutoPtr<IPackageManager> packageManager;
            hr = MsixCoreLib_CreatePackageManager(&packageManager);

            if (FAILED(hr))
            {
                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Find", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                RETURN_IF_FAILED(hr);
            }

            std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> packages;
            hr = packageManager->FindPackages(cli.GetPackageFullName(), packages);

            if (FAILED(hr))
            {
                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Find", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                RETURN_IF_FAILED(hr);
            }

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

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Find", true, WorkflowElapsedTime, L"", L"");

            return S_OK;
        }
        case OperationType::Unpack:
        {
            // Telemetry : Unpack Workflow Log
            msixmgr::TraceLogUnpackWorkflow(WorkflowId.c_str(), cli.GetPackageFilePathToInstall().c_str(),
                cli.GetFileTypeAsString().c_str(), cli.IsCreate(), cli.IsApplyACLs());

            HRESULT hr = S_OK;

            auto packageSourcePath = cli.GetPackageFilePathToInstall();
            auto unpackDestination = cli.GetUnpackDestination();
            auto rootDirectory = cli.GetRootDirectory();
            WVDFileType fileType = cli.GetFileType();
            bool createFile = cli.IsCreate();

            std::vector<std::wstring> skippedFiles;
            std::vector<std::wstring> failedPackages;
            std::vector<HRESULT> failedPackagesErrors;

            if (fileType == WVDFileType::CIM)
            {
                if (rootDirectory.empty() || fileType == WVDFileType::NotSpecified)
                {
                    std::wcout << std::endl;
                    std::wcout << "Creating a file with the -create option requires both a -rootDirectory and -fileType" << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return E_INVALIDARG;
                }
                if (!EndsWith(unpackDestination, L".cim"))
                {
                    std::wcout << std::endl;
                    std::wcout << "Invalid CIM file name. File name must have .cim file extension" << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return E_INVALIDARG;
                }

                // Create a temporary directory to unpack package(s) since we cannot unpack to the CIM directly.
                // Append long path prefix to temporary directory path to handle paths that exceed the maximum path length limit
                std::wstring currentDirectory = std::filesystem::current_path();
                std::wstring uniqueIdString;
                hr = CreateGUIDString(&uniqueIdString);

                if (FAILED(hr))
                {
                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    RETURN_IF_FAILED(hr);
                }

                std::wstring tempDirPathString = L"\\\\?\\" + currentDirectory + L"\\" + uniqueIdString;
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

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

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

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return E_UNEXPECTED;
                }

                hr = MsixCoreLib::Unpack(
                    packageSourcePath,
                    tempDirPathString,
                    cli.IsApplyACLs(),
                    cli.IsValidateSignature(),
                    skippedFiles,
                    failedPackages,
                    failedPackagesErrors);

                if (FAILED(hr))
                {
                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    RETURN_IF_FAILED(hr);
                }

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
                    std::wcout << "Creating the CIM file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateCIM << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return hrCreateCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully created the CIM file: " << unpackDestination << std::endl;
                    std::wcout << std::endl;

                    OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors);

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", true, WorkflowElapsedTime, L"", L"");
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

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                        WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                        msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                        return E_INVALIDARG;
                    }
                    else
                    {
                        if (cli.GetVHDSize() == 0)
                        {
                            std::wcout << std::endl;
                            std::wcout << "VHD size was not specified. Please provide a vhd size in MB using the -vhdSize option" << std::endl;
                            std::wcout << std::endl;

                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                            return E_INVALIDARG;
                        }

                        std::wstring driveLetter;
                        HRESULT hrCreateVHD = MsixCoreLib::CreateAndMountVHD(unpackDestination, cli.GetVHDSize(), fileType == WVDFileType::VHD,  driveLetter);
                        if (FAILED(hrCreateVHD))
                        {
                            std::wcout << std::endl;
                            std::wcout << "Creating the VHD(X) file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateVHD << std::endl;

                            if (hrCreateVHD != HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
                            {
                                // Best effort to unmount and delete the VHD file
                                if (std::filesystem::exists(std::filesystem::path(unpackDestination.c_str())))
                                {
                                    MsixCoreLib::UnmountVHD(unpackDestination);
                                    if (_wremove(unpackDestination.c_str()) != 0)
                                    {
                                        std::wcout << "Failed best-effort attempt to delete the incomplete VHD(X) file: " << unpackDestination << " Please do not use this file." << std::endl;
                                    }
                                    else
                                    {
                                        std::wcout << "Best-effort attempt to delete the incomplete VHD(X) file " << unpackDestination << " succeeded." << std::endl;
                                    }
                                }
                            }

                            std::wcout << std::endl;

                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                            return hrCreateVHD;
                        }

                        // Unpack to the mounted VHD
                        std::wstring mountedUnpackDest = driveLetter + L":\\" + cli.GetRootDirectory();
                        hr = MsixCoreLib::Unpack(
                            packageSourcePath,
                            mountedUnpackDest,
                            cli.IsApplyACLs(),
                            cli.IsValidateSignature(),
                            skippedFiles,
                            failedPackages,
                            failedPackagesErrors
                        );

                        if (FAILED(hr))
                        {
                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                            RETURN_IF_FAILED(hr);
                        }

                        HRESULT hrUnmount = MsixCoreLib::UnmountVHD(unpackDestination);
                        if (FAILED(hrUnmount))
                        {
                            std::wcout << std::endl;
                            std::wcout << "Unmounting the VHD  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateVHD << std::endl;
                            std::wcout << "Ignoring as non-fatal error.." << std::endl;
                            std::wcout << std::endl;
                        }

                        OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors);

                        std::wcout << std::endl;
                        std::wcout << "Finished unpacking packages to: " << unpackDestination << std::endl;
                        std::wcout << std::endl;

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                        WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                        msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", true, WorkflowElapsedTime, L"", L"");
                    }
                }
                else
                {
                    hr = MsixCoreLib::Unpack(
                        packageSourcePath,
                        unpackDestination,
                        cli.IsApplyACLs(),
                        cli.IsValidateSignature(),
                        skippedFiles,
                        failedPackages,
                        failedPackagesErrors);

                    if (FAILED(hr))
                    {
                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                        WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                        msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                        RETURN_IF_FAILED(hr);
                    }

                    std::wcout << std::endl;
                    std::wcout << "Finished unpacking packages to: " << unpackDestination << std::endl;
                    std::wcout << std::endl;

                    OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors);

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unpack", true, WorkflowElapsedTime, L"", L"");
                }
            }
            return hr;
        }
        case OperationType::ApplyACLs:
        {
            // Telemetry : ApplyACLs Workflow Log
            msixmgr::TraceLogApplyACLsWorkflow(WorkflowId.c_str(), cli.GetPackageFilePathToInstall().c_str());

            HRESULT hr;

            std::vector<std::wstring> packageFolders;
            packageFolders.push_back(cli.GetPackageFilePathToInstall()); // we're not actually installing anything. The API just returns the file path name we need.
            hr = MsixCoreLib::ApplyACLs(packageFolders);

            if (FAILED(hr))
            {
                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"ApplyACLs", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                RETURN_IF_FAILED(hr);
            }

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
            WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
            msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"ApplyACLs", true, WorkflowElapsedTime, L"", L"");

            return S_OK;
        }
        case OperationType::MountImage:
        {
            // Telemetry : Mount Workflow Log
            msixmgr::TraceLogMountWorkflow(WorkflowId.c_str(), cli.GetFileTypeAsString().c_str(), cli.GetMountImagePath().c_str(), cli.isMountReadOnly());

            WVDFileType fileType = cli.GetFileType();

            if (cli.GetMountImagePath().empty())
            {
                std::wcout << std::endl;
                std::wcout << "Please provide the path to the image you would like to mount." << std::endl;
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Mount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                return E_INVALIDARG;
            }

            if (fileType == WVDFileType::CIM)
            {
                std::wstring volumeId;
                HRESULT hrMountCIM = MsixCoreLib::MountCIM(cli.GetMountImagePath(), volumeId);
                if (FAILED(hrMountCIM))
                {
                    std::wcout << std::endl;
                    std::wcout << "Mounting the CIM file  " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrMountCIM << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Mount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return hrMountCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Image successfully mounted!" << std::endl;
                    std::wcout << "To examine contents in File Explorer, press Win + R and enter the following: " << std::endl;
                    std::wcout << "\\\\?\\Volume{" << volumeId << "}" << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "To unmount, run one of the followings commands: " << std::endl;
                    std::wcout << "msixmgr.exe -unmountimage -imagePath " << cli.GetMountImagePath() << " -filetype CIM" << std::endl;
                    std::wcout << "msixmgr.exe -unmountimage -volumeid " << volumeId << " -filetype CIM" << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Mount", true, WorkflowElapsedTime, L"", L"");
                }
            }
            else if (fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                std::wstring driveLetter;
                HRESULT hrMountVHD = MsixCoreLib::MountVHD(cli.GetMountImagePath(), cli.isMountReadOnly(), driveLetter);
                if (FAILED(hrMountVHD))
                {
                    std::wcout << std::endl;
                    std::wcout << "Mounting the VHD(X) file  " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrMountVHD << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Mount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return hrMountVHD;
                }
                else
                {
                    bool isVHD = cli.GetFileType() == WVDFileType::VHD;
                    std::wcout << std::endl;
                    std::wcout << "Image " << cli.GetMountImagePath() << " successfully mounted to " << driveLetter << ":\\" <<  std::endl;
                    std::wcout << "To unmount, run the following command: " << std::endl;
                    std::wcout << "msixmgr.exe -unmountimage -imagePath " << cli.GetMountImagePath() << " -filetype VHD" << (isVHD ? "" : "X") << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Mount", true, WorkflowElapsedTime, L"", L"");
                }
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -MountImage command: {VHD, VHDX, CIM}" << std::endl;
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Mount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                return ERROR_NOT_SUPPORTED;
            }
            return S_OK;
        }
        case OperationType::UnmountImage:
        {
            // Telemetry : Unmount Workflow Log
            msixmgr::TraceLogUnmountWorkflow(WorkflowId.c_str(), cli.GetFileTypeAsString().c_str(), cli.GetMountImagePath().c_str(), cli.GetVolumeId().c_str());

            WVDFileType fileType = cli.GetFileType();
            if (fileType == WVDFileType::CIM)
            {
                if (cli.GetVolumeId().empty() && cli.GetMountImagePath().empty())
                {
                    std::wcout << std::endl;
                    std::wcout << "To unmount an CIM image, please provide either the CIM file path or the volume the image was mounted to." << std::endl;
                    std::wcout << "The CIM file path can be specified using the -imagepath option." << std::endl;
                    std::wcout << "The volume can be specified using the -volumeId option." << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return E_INVALIDARG;
                }

                HRESULT hrUnmountCIM = MsixCoreLib::UnmountCIM(cli.GetMountImagePath(), cli.GetVolumeId());

                if (FAILED(hrUnmountCIM))
                {
                    std::wcout << std::endl;
                    std::wcout << "Unmounting the CIM " << " failed with HRESULT 0x" << std::hex << hrUnmountCIM << std::endl;

                    // ERROR_NOT_FOUND may be returned if only the mount image path but not the volume id was provided
                    // and msixmgr was unable to find the volume id associated with a given image path.
                    if (hrUnmountCIM == HRESULT_FROM_WIN32(ERROR_NOT_FOUND) && cli.GetVolumeId().empty())
                    {
                        std::wcout << "The error ERROR_NOT_FOUND may indicate a failure to find the volume id associated with a given image path."<< std::endl;
                        std::wcout << "Please try unmounting using the -volumeId option." << std::endl;

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                        WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                        msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                        std::wcout << std::endl;
                        return hrUnmountCIM;
                    }

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    std::wcout << std::endl;
                    return hrUnmountCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    if (!cli.GetMountImagePath().empty())
                    {
                        std::wcout << "Successfully unmounted the CIM file: " << cli.GetMountImagePath() << std::endl;
                    }
                    else
                    {
                        std::wcout << "Successfully unmounted the CIM with volume id: " << cli.GetVolumeId() << std::endl;
                    }

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", true, WorkflowElapsedTime, L"", L"");

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

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return E_INVALIDARG;
                }

                HRESULT hrUnmountVHD = MsixCoreLib::UnmountVHD(cli.GetMountImagePath());

                if (FAILED(hrUnmountVHD))
                {
                    std::wcout << std::endl;
                    std::wcout << "Unmounting the VHD " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrUnmountVHD << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

                    return hrUnmountVHD;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully unmounted the VHD " << cli.GetMountImagePath() << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                    WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                    msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", true, WorkflowElapsedTime, L"", L"");
                }
            }
            else
            {
                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -UnmountImage command: {VHD, VHDX, CIM}" << std::endl;
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&MsixMgrLoad_EndCounter);
                WorkflowElapsedTime = msixmgr::CalcWorkflowElapsedTime(MsixMgrLoad_StartCounter, MsixMgrLoad_EndCounter, MsixMgrLoad_Frequency);
                msixmgr::TraceLogWorkflow(WorkflowId.c_str(), L"Unmount", false, WorkflowElapsedTime, L"fjslf", L"dfgg");

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
    TraceLoggingUnregister(g_MsixMgrTelemetryProvider);

    return 0;
}
