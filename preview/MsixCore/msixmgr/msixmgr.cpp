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
        case OperationType::Unpack:
        {
            auto packageFilePath = cli.GetPackageFilePathToInstall();
            auto unpackDestination = cli.GetUnpackDestination();
            auto unpackDestinationUTF8 = utf16_to_utf8(unpackDestination);
            std::vector<char> destination(unpackDestinationUTF8.c_str(), unpackDestinationUTF8.c_str() + unpackDestinationUTF8.size() + 1);
            std::vector<std::wstring> packageFolders;

            MSIX_PACKUNPACK_OPTION unpackOption = MSIX_PACKUNPACK_OPTION_UNPACKWITHFLATSTRUCTURE;
            MSIX_VALIDATION_OPTION validationOption = MSIX_VALIDATION_OPTION_FULL;

            if (IsPackageFile(packageFilePath))
            {
                ComPtr<IAppxFactory> factory;
                RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, validationOption, &factory));

                ComPtr<IStream> stream;
                RETURN_IF_FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), true, &stream));

                ComPtr<IAppxPackageReader> reader;
                RETURN_IF_FAILED(factory->CreatePackageReader(stream.Get(), &reader));

                RETURN_IF_FAILED(UnpackPackageFromPackageReader(
                    unpackOption,
                    reader.Get(),
                    &destination[0]));

                if (cli.IsApplyACLs())
                {
                    PWSTR packageFullName = nullptr;

                    ComPtr<IAppxManifestReader> manifestReader;
                    RETURN_IF_FAILED(reader->GetManifest(&manifestReader));

                    ComPtr<IAppxManifestPackageId> packageId;
                    RETURN_IF_FAILED(manifestReader->GetPackageId(&packageId));

                    RETURN_IF_FAILED(packageId->GetPackageFullName(&packageFullName));

                    std::wstring packageFolderName = unpackDestination + L"\\" + packageFullName;
                    packageFolders.push_back(packageFolderName);
                }

            }
            else if (IsBundleFile(packageFilePath))
            {
                MSIX_APPLICABILITY_OPTIONS applicabilityOption = static_cast<MSIX_APPLICABILITY_OPTIONS>(MSIX_APPLICABILITY_NONE);

                ComPtr<IAppxBundleFactory> factory;
                RETURN_IF_FAILED(CoCreateAppxBundleFactoryWithHeap(MyAllocate, MyFree, validationOption, applicabilityOption, &factory));

                ComPtr<IStream> stream;
                RETURN_IF_FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), true, &stream));

                ComPtr<IAppxBundleReader> reader;
                RETURN_IF_FAILED(factory->CreateBundleReader(stream.Get(), &reader));

                RETURN_IF_FAILED(UnpackBundleFromBundleReader(
                    unpackOption,
                    reader.Get(),
                    &destination[0]));

                if (cli.IsApplyACLs())
                {
                    ComPtr<IAppxBundleManifestReader> bundleManifestReader;
                    RETURN_IF_FAILED(reader->GetManifest(&bundleManifestReader));

                    // Determine the name of the folder created for the unpacked bundle, and add this name to our list of package folders
                    PWSTR bundleFullName = nullptr;
                    ComPtr<IAppxManifestPackageId> bundleId;
                    RETURN_IF_FAILED(bundleManifestReader->GetPackageId(&bundleId));
                    RETURN_IF_FAILED(bundleId->GetPackageFullName(&bundleFullName));

                    std::wstring bundleFolderName = unpackDestination + L"\\" + bundleFullName;
                    packageFolders.push_back(bundleFolderName);

                    // Now we must determine the names of the folders created for each package in the bundle
                    // To do so, we can use the bundle reader's GetPayloadPackages API, which will tell us the names of all the package FILES 
                    // that were unpacked. While we could then create a stream for each of these files to determine their package full names, this could
                    // be potentially costly. Instead, we will use the bundle manifest reader's GetPackageInfoItems API, which will give us easy access
                    // to <package file name, package full name> pairs for ALL packages in the bundle (not necessarily the ones that were unpacked). Once
                    // we have built up a map of these file name -> full name pairs, we can quickly determine the package full name for each file name
                    // returned by GetPayloadPackages. Append the package full name to the destination to get the folder name for the unpacked package, then
                    // add this folder name to our list of package folder names.
                    std::map <std::wstring, std::wstring>  packagesMap;

                    ComPtr<IAppxBundleManifestPackageInfoEnumerator> packages;
                    ComPtr<IAppxBundleManifestPackageInfo> currentPackage;
                    RETURN_IF_FAILED(bundleManifestReader->GetPackageInfoItems(&packages));
                    BOOL hasCurrent = FALSE;
                    // Populate the packagesMap with package file name, package full name pairs
                    for (packages->GetHasCurrent(&hasCurrent); hasCurrent; packages->MoveNext(&hasCurrent))
                    {
                        RETURN_IF_FAILED(packages->GetCurrent(&currentPackage));
                        ComPtr<IAppxManifestPackageId> manifestPackageID;
                        RETURN_IF_FAILED(currentPackage->GetPackageId(&manifestPackageID));

                        PWSTR packageFullName = nullptr;
                        RETURN_IF_FAILED(manifestPackageID->GetPackageFullName(&packageFullName));

                        PWSTR packageFileName = nullptr;
                        RETURN_IF_FAILED(currentPackage->GetFileName(&packageFileName));

                        std::wstring stdPackageFileName = packageFileName;
                        std::wstring stdPackageFullName = packageFullName;

                        packagesMap[stdPackageFileName] = stdPackageFullName;
                    }

                    // Use GetPayloadPackages to enumerate over the package files that actually got unpacked
                    // after applicability logic was applied. 
                    hasCurrent = false;
                    ComPtr<IAppxFilesEnumerator> packageFilesEnumerator;
                    RETURN_IF_FAILED(reader->GetPayloadPackages(&packageFilesEnumerator));
                    RETURN_IF_FAILED(packageFilesEnumerator->GetHasCurrent(&hasCurrent));
                    while (hasCurrent)
                    {
                        ComPtr<IAppxFile> file;
                        RETURN_IF_FAILED(packageFilesEnumerator->GetCurrent(&file));

                        PWSTR packageFileName = nullptr;
                        RETURN_IF_FAILED(file->GetName(&packageFileName));

                        std::wstring stdPackageFileName = packageFileName;
                        auto packageFullName = packagesMap[packageFileName];
                        std::wstring packageFolderName = unpackDestination + L"\\" + packageFullName;
                        packageFolders.push_back(packageFolderName);

                        RETURN_IF_FAILED(packageFilesEnumerator->MoveNext(&hasCurrent));
                    }
                }

            }
            else
            {
                std::wcout << "Invalid package path " << packageFilePath << " specified. Please confirm the given package path is an .appx, .appxbundle, .msix, or .msixbundle file" << std::endl;
                return E_INVALIDARG;
            }

            if (cli.IsApplyACLs())
            {
                std::unique_ptr<HMODULE> applyACLsDll;
                *applyACLsDll = (LoadLibrary(L"applyacls.dll"));
                if (applyACLsDll == nullptr)
                {
                    std::wcout << "Failed to load applyacls.dll. Please confirm the dll is next to this exe" << std::endl;
                }

                typedef HRESULT(STDMETHODCALLTYPE *APPLYACLSTOPACKAGEFOLDER)(PCWSTR folderPath);

                APPLYACLSTOPACKAGEFOLDER ApplyACLsToPackageFolder =
                    reinterpret_cast<APPLYACLSTOPACKAGEFOLDER>
                    (GetProcAddress(*applyACLsDll, "ApplyACLsToPackageFolder"));

                for (auto folder : packageFolders)
                {
                    std::wcout << folder << std::endl;
                    RETURN_IF_FAILED(ApplyACLsToPackageFolder(folder.c_str()));
                }

                return S_OK;
            }

        }
        case OperationType::ApplyACLs:
        {
            // Call helper function to apply ACLs
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
