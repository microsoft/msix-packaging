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
#include "msixmgrTraceLogging.hpp"
#include "ErrorMessageHelper.hpp"

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
    _In_ std::vector<HRESULT> failedPackagesErrors,
    _In_ CommandLineInterface cli,
    _In_ std::wstring &errorDesc)
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

        errorDesc += L" " + std::to_wstring(failedPackages.size()) + L" Packages Failed.";

        for (int i = 0; i < failedPackages.size(); i++)
        {
            HRESULT hr = failedPackagesErrors.at(i);

            std::wstring errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hr);
            errorDesc += L" (" + std::to_wstring(i+1) + L") " + L"HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hr);

            std::wcout << L"Failed with HRESULT 0x" << std::hex << hr << L" when trying to unpack " << failedPackages.at(i) << std::endl;

            if (hr == static_cast<HRESULT>(MSIX::Error::CertNotTrusted))
            {
                std::wcout << L"Please confirm that the certificate has been installed for this package" << std::endl;
                errorDesc += L" Please confirm that the certificate has been installed for this package.";
            }
            else if (hr == static_cast<HRESULT>(MSIX::Error::FileWrite))
            {
                if ((cli.GetFileType() == WVDFileType::VHD || cli.GetFileType() == WVDFileType::VHDX) && cli.GetVHDSize() == 0)
                {
                    std::wcout << L"The tool encountered a file write error. Since VHDSize parameter was not specified, the tool tried with default VHDSize of 4 times the size of the package and operation got failed. Please try again by specifying the VHDSize (larger than 4 times), as file write errors may be caused by insufficient disk space." << std::endl;
                    errorDesc += L" The tool encountered a file write error. Since VHDSize parameter was not specified, the tool tried with default VHDSize of 4 times the size of the package and operation got failed. Please try again by specifying the VHDSize (larger than 4 times), as file write errors may be caused by insufficient disk space.";
                }
                else
                {
                    std::wcout << L"The tool encountered a file write error. If you are unpacking to a VHD, please try again with a larger VHD, as file write errors may be caused by insufficient disk space." << std::endl;
                    errorDesc += L" The tool encountered a file write error. If you are unpacking to a VHD, please try again with a larger VHD, as file write errors may be caused by insufficient disk space.";
                }
            }
            else if (hr == E_INVALIDARG)
            {
                std::wcout << "Please confirm the given package path is an .appx, .appxbundle, .msix, or .msixbundle file" << std::endl;
                errorDesc += L" Please confirm the given package path is an .appx, .appxbundle, .msix, or .msixbundle file.";
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
    std::wstring workflowId, sourceApplicationId, correlationId, errorCode = L"", errorDesc = L"";
    double workflowElapsedTime;
    LARGE_INTEGER msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency;
    QueryPerformanceFrequency(&msixMgrLoad_Frequency);

    // Creating workflowId for tracking the Workflows
    workflowId = msixmgrTraceLogging::CreateWorkflowId();
    sourceApplicationId = cli.GetSourceApplicationId();    // SourceApplicationId indicates which application is executing the MSIXMGR commands. Useful during integration of MSIXMGR with other Tools.
    correlationId = cli.GetCorrelationId();    // CorrelationId can be used to establish a correlation between MSIXMGR's Workflow and the parent application's Workflow, when MSIXMGR is being used in integration with any other Tool.

    // Telemetry : Session Log
    msixmgrTraceLogging::TraceLogSession(workflowId.c_str(), sourceApplicationId.c_str(), correlationId.c_str());

    QueryPerformanceCounter(&msixMgrLoad_StartCounter);

    if (SUCCEEDED(hrCreateRequest))
    {
        switch (cli.GetOperationType())
        {
        case OperationType::Add:
        {
            // Telemetry : Add Workflow Log
            msixmgrTraceLogging::TraceLogAddWorkflow(workflowId.c_str(), msixmgrTraceLogging::ExtractPackageNameFromFilePath(cli.GetPackageFilePathToInstall()).c_str());

            HRESULT hrCreatePackageManager;

            AutoPtr<IPackageManager> packageManager;
            hrCreatePackageManager = MsixCoreLib_CreatePackageManager(&packageManager);

            if (FAILED(hrCreatePackageManager))
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrCreatePackageManager);
                errorDesc = L"Failed creation of Package Manager Object. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreatePackageManager);


                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                RETURN_IF_FAILED(hrCreatePackageManager);
            }

            if (cli.IsQuietMode())
            {
                if (!isAdmin)
                {
                    errorCode = L"";
                    errorDesc = L"Relaunching as Admin.";

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    RelaunchAsAdmin(argc, argv);
                    return 0;
                }
                HRESULT hrAddPackage = packageManager->AddPackage(cli.GetPackageFilePathToInstall(), DeploymentOptions::None);
                if (FAILED(hrAddPackage))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrAddPackage);
                    errorDesc = L"Failed Add Package Operation. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrAddPackage);

                    std::wcout << GetStringResource(IDS_STRING_FAILED_REQUEST) << " " << std::hex << hrAddPackage << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrAddPackage);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

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
                            HRESULT hrHRESULTFromWin32;
                            hrHRESULTFromWin32 = HRESULT_FROM_WIN32(GetLastError());

                            errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrHRESULTFromWin32);
                            errorDesc = L"Failed Add Package Operation. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrHRESULTFromWin32);

                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                            return hrHRESULTFromWin32;
                        }

                        protocol.append(path);
                    }
                    ShellExecuteW(nullptr, L"Open", protocol.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                }
                else
                {
                    if (!isAdmin)
                    {
                        errorCode = L"";
                        errorDesc = L"Relaunching as Admin.";

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                        workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                        msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                        RelaunchAsAdmin(argc, argv);
                        return 0;
                    }
                    auto ui = new UI(packageManager, cli.GetPackageFilePathToInstall(), UIType::InstallUIAdd);
                    HRESULT hrShowUI;
                    hrShowUI = ui->ShowUI();

                    if (FAILED(hrShowUI))
                    {
                        errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrShowUI);
                        errorDesc = L"Failed Show UI Operation for Add Package Operation. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrShowUI);

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                        workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                        msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                        RETURN_IF_FAILED(hrShowUI);
                    }
                }
            }

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");

            break;
        }
        case OperationType::Remove:
        {
            // Telemetry : Remove Workflow Log
            msixmgrTraceLogging::TraceLogRemoveWorkflow(workflowId.c_str(), msixmgrTraceLogging::ExtractPackageNameFromFilePath(cli.GetPackageFullName()).c_str());

            if (!isAdmin)
            {
                errorCode = L"";
                errorDesc = L"Relaunching as Admin.";

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                RelaunchAsAdmin(argc, argv);
                return 0;
            }
            FreeConsole();
            AutoPtr<IPackageManager> packageManager;
            HRESULT hrCreatePackageManager;
            hrCreatePackageManager = MsixCoreLib_CreatePackageManager(&packageManager);

            if (FAILED(hrCreatePackageManager))
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrCreatePackageManager);
                errorDesc = L"Failed creation of Package Manager Object. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreatePackageManager);

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                RETURN_IF_FAILED(hrCreatePackageManager);
            }

            auto packageFullName = cli.GetPackageFullName();
            HRESULT hrRemovePackage = packageManager->RemovePackage(packageFullName);
            if (FAILED(hrRemovePackage))
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrRemovePackage);
                errorDesc = L"Failed Remove Package Operation. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrRemovePackage);

                std::wcout << GetStringResource(IDS_STRING_FAILED_REQUEST) << " " << std::hex << hrRemovePackage << std::endl;
                std::wcout << std::endl;
                std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrRemovePackage);
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                return hrRemovePackage;
            }

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");

            break;
        }
        case OperationType::FindPackage:
        {
            // Telemetry : Find Workflow Log
            msixmgrTraceLogging::TraceLogFindWorkflow(workflowId.c_str(), msixmgrTraceLogging::ExtractPackageNameFromFilePath(cli.GetPackageFullName()).c_str());

            HRESULT hrCreatePackageManager;
            AutoPtr<IPackageManager> packageManager;
            hrCreatePackageManager = MsixCoreLib_CreatePackageManager(&packageManager);

            if (FAILED(hrCreatePackageManager))
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrCreatePackageManager);
                errorDesc = L"Failed creation of Package Manager Object. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreatePackageManager);

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                RETURN_IF_FAILED(hrCreatePackageManager);
            }

            std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> packages;
            HRESULT hrFindPackage;
            hrFindPackage = packageManager->FindPackages(cli.GetPackageFullName(), packages);

            if (FAILED(hrFindPackage))
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrFindPackage);
                errorDesc = L"Failed Find Package Operation. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrFindPackage);

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                RETURN_IF_FAILED(hrFindPackage);
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
            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");

            return S_OK;
        }
        case OperationType::Unpack:
        {
            // Telemetry : Unpack Workflow Log
            msixmgrTraceLogging::TraceLogUnpackWorkflow(workflowId.c_str(), msixmgrTraceLogging::ExtractPackageNameFromFilePath(cli.GetPackageFilePathToInstall()).c_str(),
                cli.GetFileTypeAsString().c_str(), cli.GetVHDSize(), cli.IsCreate(), cli.IsApplyACLs());

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
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_INVALIDARG);
                    errorDesc = L"Creating a file with the -create option requires both a -rootDirectory and -fileType. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);

                    std::wcout << std::endl;
                    std::wcout << "Creating a file with the -create option requires both a -rootDirectory and -fileType." << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return E_INVALIDARG;
                }
                if (!EndsWith(unpackDestination, L".cim"))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_INVALIDARG);
                    errorDesc = L"Invalid CIM file name. File name must have .cim file extension. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);

                    std::wcout << std::endl;
                    std::wcout << "Invalid CIM file name. File name must have .cim file extension." << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return E_INVALIDARG;
                }

                // Create a temporary directory to unpack package(s) since we cannot unpack to the CIM directly.
                // Append long path prefix to temporary directory path to handle paths that exceed the maximum path length limit
                std::wstring currentDirectory = std::filesystem::current_path();
                std::wstring uniqueIdString;
                HRESULT hrCreateGUIDString = CreateGUIDString(&uniqueIdString);

                if (FAILED(hrCreateGUIDString))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrCreateGUIDString);
                    errorDesc = L"Failed UniqueGuid creation for tempDirPathString for CIM file. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreateGUIDString);

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    RETURN_IF_FAILED(hrCreateGUIDString);
                }

                std::wstring tempDirPathString = L"\\\\?\\" + currentDirectory + L"\\" + uniqueIdString;
                std::filesystem::path tempDirPath(tempDirPathString);

                std::error_code createDirectoryErrorCode;
                bool createTempDirResult = std::filesystem::create_directory(tempDirPath, createDirectoryErrorCode);

                // Since we're using a GUID, this should almost never happen
                if (!createTempDirResult)
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_UNEXPECTED);
                    errorDesc = L"Failed to create temp directory. This may occur when the directory path already exists. Please try again. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_UNEXPECTED);

                    std::wcout << std::endl;
                    std::wcout << "Failed to create temp directory " << tempDirPathString << std::endl;
                    std::wcout << "This may occur when the directory path already exists. Please try again."  << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_UNEXPECTED);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return E_UNEXPECTED;
                }
                if (createDirectoryErrorCode.value() != 0)
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_UNEXPECTED);
                    errorDesc = L"Creation of temp directory failed with error: " + std::to_wstring(createDirectoryErrorCode.value()) + L". Error Message: " + utf8_to_utf16(createDirectoryErrorCode.message()) + L". Please try again. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_UNEXPECTED);

                    // Again, we expect that the creation of the temp directory will fail very rarely. Output the exception
                    // and have the user try again.
                    std::wcout << std::endl;
                    std::wcout << "Creation of temp directory " << tempDirPathString << " failed with error: " << createDirectoryErrorCode.value() << std::endl;
                    std::cout << "Error message: " << createDirectoryErrorCode.message() << std::endl;
                    std::wcout << "Please try again." << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_UNEXPECTED);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return E_UNEXPECTED;
                }

                HRESULT hrUnpackToTempDir = MsixCoreLib::Unpack(
                    packageSourcePath,
                    tempDirPathString,
                    cli.IsApplyACLs(),
                    cli.IsValidateSignature(),
                    skippedFiles,
                    failedPackages,
                    failedPackagesErrors);

                if (FAILED(hrUnpackToTempDir))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrUnpackToTempDir);
                    errorDesc = L"Failed to Unpack in Temp Directory for CIM flow. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnpackToTempDir);

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    RETURN_IF_FAILED(hrUnpackToTempDir);
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
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrCreateCIM);
                    errorDesc = L"Creating the CIM file failed. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreateCIM);

                    std::wcout << std::endl;
                    std::wcout << "Creating the CIM file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateCIM << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreateCIM);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return hrCreateCIM;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully created the CIM file: " << unpackDestination << std::endl;
                    std::wcout << std::endl;

                    OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors, cli, errorDesc);

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", errorDesc.c_str());

                    return failedPackagesErrors.empty() ? S_OK : failedPackagesErrors.back();
                }
                 
            }
            // UnpackDestinationFileType::NotSpecified is only valid if unpacking to an existing VHD
            else if (fileType == WVDFileType::NotSpecified || fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                if (createFile)
                {
                    if (!(EndsWith(unpackDestination, L".vhd") || (EndsWith(unpackDestination, L".vhdx"))))
                    {
                        errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_INVALIDARG);
                        errorDesc = L"Invalid VHD file name. File name must have .vhd or .vhdx file extension. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);

                        std::wcout << std::endl;
                        std::wcout << "Invalid VHD file name. File name must have .vhd or .vhdx file extension." << std::endl;
                        std::wcout << std::endl;
                        std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);
                        std::wcout << std::endl;

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                        workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                        msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                        return E_INVALIDARG;
                    }
                    else
                    {
                        ULONGLONG vhdSize = cli.GetVHDSize();
                        if (vhdSize == 0)
                        {
                            std::uintmax_t size = std::filesystem::file_size(packageSourcePath);
                            size = size / (1024 * 1024); //converting bytes into MBs
                            vhdSize = size * 4; //assuming minimum VHD size to be 4x of source package size in MB
                        }

                        std::wstring driveLetter;
                        HRESULT hrCreateVHD = MsixCoreLib::CreateAndMountVHD(unpackDestination, vhdSize, fileType == WVDFileType::VHD,  driveLetter);
                        if (FAILED(hrCreateVHD))
                        {
                            errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrCreateVHD);
                            errorDesc = L"Creation of VHD(X) file failed. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreateVHD);

                            std::wcout << std::endl;
                            std::wcout << "Creating the VHD(X) file  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateVHD << std::endl;
                            std::wcout << std::endl;
                            std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrCreateVHD);
                            std::wcout << std::endl;

                            if (hrCreateVHD != HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
                            {
                                // Best effort to unmount and delete the VHD file
                                if (std::filesystem::exists(std::filesystem::path(unpackDestination.c_str())))
                                {
                                    MsixCoreLib::UnmountVHD(unpackDestination);
                                    if (_wremove(unpackDestination.c_str()) != 0)
                                    {
                                        errorDesc += L"Failed best-effort attempt to delete the incomplete VHD(X) file. Please do not use this file.";
                                        std::wcout << "Failed best-effort attempt to delete the incomplete VHD(X) file: " << unpackDestination << " Please do not use this file." << std::endl;
                                    }
                                    else
                                    {
                                        errorDesc += L"Best-effort attempt to delete the incomplete VHD(X) file succeeded.";
                                        std::wcout << "Best-effort attempt to delete the incomplete VHD(X) file " << unpackDestination << " succeeded." << std::endl;
                                    }
                                }
                            }

                            std::wcout << std::endl;

                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                            return hrCreateVHD;
                        }

                        // Unpack to the mounted VHD
                        std::wstring mountedUnpackDest = driveLetter + L":\\" + cli.GetRootDirectory();
                        HRESULT hrUnpackToVHD = MsixCoreLib::Unpack(
                            packageSourcePath,
                            mountedUnpackDest,
                            cli.IsApplyACLs(),
                            cli.IsValidateSignature(),
                            skippedFiles,
                            failedPackages,
                            failedPackagesErrors
                        );

                        if (FAILED(hrUnpackToVHD))
                        {
                            errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrUnpackToVHD);
                            errorDesc = L"Failed unpack to the mounted vhd(x). HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnpackToVHD);

                            // Telemetry : Workflow Log
                            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                            RETURN_IF_FAILED(hrUnpackToVHD);
                        }

                        HRESULT hrUnmount = MsixCoreLib::UnmountVHD(unpackDestination);
                        if (FAILED(hrUnmount))
                        {
                            errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrUnmount);
                            errorDesc = L"Successful Unpack to mounted vhd(x). Unmounting the VHD failed. Ignoring as non-fatal error. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnmount);

                            std::wcout << std::endl;
                            std::wcout << "Unmounting the VHD  " << unpackDestination << " failed with HRESULT 0x" << std::hex << hrCreateVHD << std::endl;
                            std::wcout << "Ignoring as non-fatal error.." << std::endl;
                            std::wcout << std::endl;
                            std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnmount);
                            std::wcout << std::endl;
                        }

                        OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors, cli, errorDesc);

                        std::wcout << std::endl;
                        std::wcout << "Finished unpacking packages to: " << unpackDestination << std::endl;
                        std::wcout << std::endl;

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                        workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                        msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", errorDesc.c_str());

                        return failedPackagesErrors.empty() ? S_OK : failedPackagesErrors.back();
                    }
                }
                else
                {
                    HRESULT hrUnpackToFolder = MsixCoreLib::Unpack(
                        packageSourcePath,
                        unpackDestination,
                        cli.IsApplyACLs(),
                        cli.IsValidateSignature(),
                        skippedFiles,
                        failedPackages,
                        failedPackagesErrors);

                    if (FAILED(hrUnpackToFolder))
                    {
                        errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrUnpackToFolder);
                        errorDesc = L"Failed unpack to the given folder or given VHD(X). HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnpackToFolder);

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                        workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                        msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                        RETURN_IF_FAILED(hrUnpackToFolder);
                    }

                    std::wcout << std::endl;
                    std::wcout << "Finished unpacking packages to: " << unpackDestination << std::endl;
                    std::wcout << std::endl;

                    OutputUnpackFailures(packageSourcePath, skippedFiles, failedPackages, failedPackagesErrors, cli, errorDesc);

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", errorDesc.c_str());

                    return failedPackagesErrors.empty() ? S_OK : failedPackagesErrors.back();
                }
            }
            return S_OK;
        }
        case OperationType::ApplyACLs:
        {
            // Telemetry : ApplyACLs Workflow Log
            msixmgrTraceLogging::TraceLogApplyACLsWorkflow(workflowId.c_str(), msixmgrTraceLogging::ExtractPackageNameFromFilePath(cli.GetPackageFilePathToInstall()).c_str());

            HRESULT hrApplyACLs;

            std::vector<std::wstring> packageFolders;
            packageFolders.push_back(cli.GetPackageFilePathToInstall()); // we're not actually installing anything. The API just returns the file path name we need.
            hrApplyACLs = MsixCoreLib::ApplyACLs(packageFolders);

            if (FAILED(hrApplyACLs))
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrApplyACLs);
                errorDesc = L"Failed ApplyACLs Operation. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrApplyACLs);

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                RETURN_IF_FAILED(hrApplyACLs);
            }

            // Telemetry : Workflow Log
            QueryPerformanceCounter(&msixMgrLoad_EndCounter);
            workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
            msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");

            return S_OK;
        }
        case OperationType::MountImage:
        {
            // Telemetry : Mount Workflow Log
            msixmgrTraceLogging::TraceLogMountWorkflow(workflowId.c_str(), cli.GetFileTypeAsString().c_str());

            WVDFileType fileType = cli.GetFileType();

            if (cli.GetMountImagePath().empty())
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_INVALIDARG);
                errorDesc = L"Please provide the path to the image you would like to mount. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);

                std::wcout << std::endl;
                std::wcout << "Please provide the path to the image you would like to mount." << std::endl;
                std::wcout << std::endl;
                std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                return E_INVALIDARG;
            }

            if (fileType == WVDFileType::CIM)
            {
                std::wstring volumeId;
                HRESULT hrMountCIM = MsixCoreLib::MountCIM(cli.GetMountImagePath(), volumeId);
                if (FAILED(hrMountCIM))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrMountCIM);
                    errorDesc = L"Mounting the CIM file failed. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrMountCIM);

                    std::wcout << std::endl;
                    std::wcout << "Mounting the CIM file  " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrMountCIM << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrMountCIM);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

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
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");
                }
            }
            else if (fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                std::wstring driveLetter;
                HRESULT hrMountVHD = MsixCoreLib::MountVHD(cli.GetMountImagePath(), cli.isMountReadOnly(), driveLetter);
                if (FAILED(hrMountVHD))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrMountVHD);
                    errorDesc = L"Mounting the VHD(X) file failed. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrMountVHD);

                    std::wcout << std::endl;
                    std::wcout << "Mounting the VHD(X) file  " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrMountVHD << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrMountVHD);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

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
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");
                }
            }
            else
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(ERROR_NOT_SUPPORTED);
                errorDesc = L"Please specify one of the following supported file types for the -MountImage command: {VHD, VHDX, CIM}. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(ERROR_NOT_SUPPORTED);

                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -MountImage command: {VHD, VHDX, CIM}" << std::endl;
                std::wcout << std::endl;
                std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(ERROR_NOT_SUPPORTED);
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                return ERROR_NOT_SUPPORTED;
            }
            return S_OK;
        }
        case OperationType::UnmountImage:
        {
            // Telemetry : Unmount Workflow Log
            msixmgrTraceLogging::TraceLogUnmountWorkflow(workflowId.c_str(), cli.GetFileTypeAsString().c_str());

            WVDFileType fileType = cli.GetFileType();
            if (fileType == WVDFileType::CIM)
            {
                if (cli.GetVolumeId().empty() && cli.GetMountImagePath().empty())
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_INVALIDARG);
                    errorDesc = L"To unmount an CIM image, please provide either the CIM file path or the volume the image was mounted to. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);

                    std::wcout << std::endl;
                    std::wcout << "To unmount an CIM image, please provide either the CIM file path or the volume the image was mounted to." << std::endl;
                    std::wcout << "The CIM file path can be specified using the -imagepath option." << std::endl;
                    std::wcout << "The volume can be specified using the -volumeId option." << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return E_INVALIDARG;
                }

                HRESULT hrUnmountCIM = MsixCoreLib::UnmountCIM(cli.GetMountImagePath(), cli.GetVolumeId());

                if (FAILED(hrUnmountCIM))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrUnmountCIM);
                    errorDesc = L"Unmounting the CIM file failed. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnmountCIM);

                    std::wcout << std::endl;
                    std::wcout << "Unmounting the CIM " << " failed with HRESULT 0x" << std::hex << hrUnmountCIM << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnmountCIM);
                    std::wcout << std::endl;

                    // ERROR_NOT_FOUND may be returned if only the mount image path but not the volume id was provided
                    // and msixmgr was unable to find the volume id associated with a given image path.
                    if (hrUnmountCIM == HRESULT_FROM_WIN32(ERROR_NOT_FOUND) && cli.GetVolumeId().empty())
                    {
                        errorDesc += L"The error ERROR_NOT_FOUND may indicate a failure to find the volume id associated with a given image path. Please try unmounting using the -volumeId option.";

                        std::wcout << "The error ERROR_NOT_FOUND may indicate a failure to find the volume id associated with a given image path."<< std::endl;
                        std::wcout << "Please try unmounting using the -volumeId option." << std::endl;

                        // Telemetry : Workflow Log
                        QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                        workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                        msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                        std::wcout << std::endl;
                        return hrUnmountCIM;
                    }

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

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
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");

                    std::wcout << std::endl;
                }
            }
            else if (fileType == WVDFileType::VHD || fileType == WVDFileType::VHDX)
            {
                if (cli.GetMountImagePath().empty())
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(E_INVALIDARG);
                    errorDesc = L"Please provide the path to the image you would like to unmount. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);

                    std::wcout << std::endl;
                    std::wcout << "Please provide the path to the image you would like to unmount." << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(E_INVALIDARG);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return E_INVALIDARG;
                }

                HRESULT hrUnmountVHD = MsixCoreLib::UnmountVHD(cli.GetMountImagePath());

                if (FAILED(hrUnmountVHD))
                {
                    errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(hrUnmountVHD);
                    errorDesc = L"Unmounting the VHD file failed. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnmountVHD);

                    std::wcout << std::endl;
                    std::wcout << "Unmounting the VHD " << cli.GetMountImagePath() << " failed with HRESULT 0x" << std::hex << hrUnmountVHD << std::endl;
                    std::wcout << std::endl;
                    std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(hrUnmountVHD);
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

                    return hrUnmountVHD;
                }
                else
                {
                    std::wcout << std::endl;
                    std::wcout << "Successfully unmounted the VHD " << cli.GetMountImagePath() << std::endl;
                    std::wcout << std::endl;

                    // Telemetry : Workflow Log
                    QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                    workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                    msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), true, workflowElapsedTime, L"", L"");
                }
            }
            else
            {
                errorCode = msixmgrTraceLogging::GetErrorCodeFromHRESULT(ERROR_NOT_SUPPORTED);
                errorDesc = L"Please specify one of the following supported file types for the -UnmountImage command: {VHD, VHDX, CIM}. HRESULT " + errorCode + L". HRESULT Desc - " + ErrorMessageHelper::GetErrorMessageFromHRESULT(ERROR_NOT_SUPPORTED);

                std::wcout << std::endl;
                std::wcout << "Please specify one of the following supported file types for the -UnmountImage command: {VHD, VHDX, CIM}" << std::endl;
                std::wcout << std::endl;
                std::wcout << "HRESULT - " << errorCode << ". HRESULT Desc - " << ErrorMessageHelper::GetErrorMessageFromHRESULT(ERROR_NOT_SUPPORTED);
                std::wcout << std::endl;

                // Telemetry : Workflow Log
                QueryPerformanceCounter(&msixMgrLoad_EndCounter);
                workflowElapsedTime = msixmgrTraceLogging::CalcWorkflowElapsedTime(msixMgrLoad_StartCounter, msixMgrLoad_EndCounter, msixMgrLoad_Frequency);
                msixmgrTraceLogging::TraceLogWorkflow(workflowId.c_str(), cli.GetOperationTypeAsString().c_str(), false, workflowElapsedTime, errorCode.c_str(), errorDesc.c_str());

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
