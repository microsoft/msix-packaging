#include "PackageManager.hpp"
#include "MsixRequest.hpp"
#include "Constants.hpp"
#include "PopulatePackageInfo.hpp"
#include "MsixTraceLoggingProvider.hpp"
#include "Database.hpp"
#include <experimental/filesystem>
#include <thread>

using namespace std;
using namespace MsixCoreLib;

PackageManager::PackageManager()
{
}

shared_ptr<IMsixResponse> PackageManager::AddPackageAsync(const wstring & packageFilePath, DeploymentOptions options, function<void(const IMsixResponse&)> callback)
{
    ComPtr<IStream> packageStream;
    if (FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), /*forRead */ true, &packageStream)))
    {
        return nullptr;
    }

    return AddPackageAsync(packageStream.Get(), options, callback);
}

shared_ptr<IMsixResponse> PackageManager::AddPackageAsync(IStream * packageStream, DeploymentOptions options, function<void(const IMsixResponse&)> callback)
{
    MsixRequest * impl;
    HRESULT hr = (MsixRequest::Make(OperationType::Add, packageStream, L"", MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(hr))
    {
        return nullptr;
    }

    if (callback != nullptr)
    {
        impl->GetMsixResponse()->SetCallback(callback);
    }

    auto t = thread([&](MsixRequest* msixRequest) {
        msixRequest->ProcessRequest();
        delete msixRequest;
        msixRequest = nullptr;
        }, impl);
    t.detach();
    return impl->GetMsixResponse();
}

HRESULT PackageManager::AddPackage(IStream * packageStream, DeploymentOptions options)
{
    AutoPtr<MsixRequest> impl;
    RETURN_IF_FAILED(MsixRequest::Make(OperationType::Add, packageStream, L"", MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    
    RETURN_IF_FAILED(impl->ProcessRequest());
    return S_OK;
}

HRESULT PackageManager::AddPackage(const wstring & packageFilePath, DeploymentOptions options)
{
    ComPtr<IStream> packageStream;
    auto res = CreateStreamOnFileUTF16(packageFilePath.c_str(), /*forRead */ true, &packageStream);
    if (FAILED(res))
    {
        return res;
    }

    return AddPackage(packageStream.Get(), options);
}

shared_ptr<IMsixResponse> PackageManager::RemovePackageAsync(const wstring & packageFullName, function<void(const IMsixResponse&)> callback)
{
    MsixRequest* impl;
    HRESULT hr = (MsixRequest::Make(OperationType::Remove, nullptr, packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(hr))
    {
        return nullptr;
    }

    if (callback != nullptr)
    {
        impl->GetMsixResponse()->SetCallback(callback);
    }

    auto t = thread([&](MsixRequest* msixRequest) {
        msixRequest->ProcessRequest();
        delete msixRequest;
        msixRequest = nullptr;
        }, impl);
    t.detach();
    return impl->GetMsixResponse();
}

HRESULT PackageManager::RemovePackage(const wstring & packageFullName)
{
    AutoPtr<MsixRequest> impl;
    RETURN_IF_FAILED(MsixRequest::Make(OperationType::Remove, nullptr, packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    
    RETURN_IF_FAILED(impl->ProcessRequest());
    return S_OK;
}

HRESULT PackageManager::GetPackageInfo(const wstring & directoryPath, shared_ptr<IInstalledPackage> & installedPackage)
{
    std::shared_ptr<InstalledPackage> packageInfo;
    RETURN_IF_FAILED(PopulatePackageInfo::GetPackageInfoFromManifest(directoryPath.c_str(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo));
    
    installedPackage = std::dynamic_pointer_cast<IInstalledPackage>(packageInfo);
    return S_OK;
}

HRESULT PackageManager::FindPackage(const wstring & packageFullName, shared_ptr<IInstalledPackage>& installedPackage)
{
    auto filemapping = FilePathMappings::GetInstance();
    RETURN_IF_FAILED(filemapping.GetInitializationResult());
    
    wstring msixCoreDirectory = filemapping.GetMsixCoreDirectory();
    wstring packageDirectoryPath = msixCoreDirectory + packageFullName;
    RETURN_IF_FAILED(GetPackageInfo(packageDirectoryPath, installedPackage));
    return S_OK;
}

HRESULT PackageManager::FindPackageByFamilyName(const wstring & packageFamilyName, shared_ptr<IInstalledPackage>& installedPackage)
{
    auto filemapping = FilePathMappings::GetInstance();
    RETURN_IF_FAILED(filemapping.GetInitializationResult());
    auto msixCoreDirectory = filemapping.GetMsixCoreDirectory();

    std::vector<std::wstring> packageFullNames;
    RETURN_IF_FAILED(Database::FindPackagesForCurrentUser(packageFullNames));
    for (auto&packageFullName : packageFullNames)
    {
        auto installedAppFamilyName = GetFamilyNameFromFullName(packageFullName);
        if (CaseInsensitiveEquals(installedAppFamilyName, packageFamilyName))
        {
            wstring packageDirectoryPath = msixCoreDirectory + packageFullName;
            RETURN_IF_FAILED(GetPackageInfo(packageDirectoryPath, installedPackage));
            return S_OK;
        }
    }
    return S_OK;
}

HRESULT PackageManager::FindPackages(unique_ptr<vector<shared_ptr<IInstalledPackage>>> & installedPackages)
{
    auto packages = std::make_unique<std::vector<shared_ptr<IInstalledPackage>>>();

    auto filemapping = FilePathMappings::GetInstance();
    RETURN_IF_FAILED(filemapping.GetInitializationResult());
    wstring msixCoreDirectory = filemapping.GetMsixCoreDirectory();

    std::vector<std::wstring> packageFullNames;
    RETURN_IF_FAILED(Database::FindPackagesForCurrentUser(packageFullNames));
    for (auto&packageFullName : packageFullNames)
    {
        wstring packageDirectoryPath = msixCoreDirectory + packageFullName;
        shared_ptr<IInstalledPackage> packageInfo;
        RETURN_IF_FAILED(GetPackageInfo(packageDirectoryPath, packageInfo));

        packages->push_back(packageInfo);
    }
    
    installedPackages.swap(packages);
    return S_OK;
}

HRESULT PackageManager::GetMsixPackageInfo(const wstring & msixFullPath, shared_ptr<IPackage> & package)
{
    auto filemapping = FilePathMappings::GetInstance();
    RETURN_IF_FAILED(filemapping.GetInitializationResult());
    
    shared_ptr<Package> packageInfo;
    ComPtr<IStream> packageStream;
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(msixFullPath.c_str(), /*forRead */ true, &packageStream));
    RETURN_IF_FAILED(PopulatePackageInfo::GetPackageInfoFromPackage(packageStream.Get(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo));
    
    package = dynamic_pointer_cast<IPackage>(packageInfo);
    return S_OK;
}