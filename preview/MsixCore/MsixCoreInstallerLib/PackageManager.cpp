#include "PackageManager.hpp"
#include "MsixRequest.hpp"
#include "Constants.hpp"
#include "PopulatePackageInfo.hpp"
#include <experimental/filesystem>
#include <thread>

using namespace std;
using namespace MsixCoreLib;

PackageManager::PackageManager()
{
}

shared_ptr<IMsixResponse> PackageManager::AddPackageAsync(const wstring & packageFilePath, DeploymentOptions options, function<void(const IMsixResponse&)> callback)
{
    MsixRequest * impl;
    auto res = (MsixRequest::Make(OperationType::Add, packageFilePath, L"", MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(res))
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

HRESULT PackageManager::AddPackage(const wstring & packageFilePath, DeploymentOptions options)
{
    AutoPtr<MsixRequest> impl;
    auto res = (MsixRequest::Make(OperationType::Add, packageFilePath, L"", MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(res))
    {
        return res;
    }
    return impl->ProcessRequest();
}

shared_ptr<IMsixResponse> PackageManager::RemovePackageAsync(const wstring & packageFullName, function<void(const IMsixResponse&)> callback)
{
    MsixRequest* impl;
    auto res = (MsixRequest::Make(OperationType::Remove, L"", packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(res))
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
    auto res = (MsixRequest::Make(OperationType::Remove, L"", packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(res))
    {
        return res;
    }
    return impl->ProcessRequest();
}
shared_ptr<IInstalledPackage> PackageManager::GetPackageInfo(const wstring & msixCoreDirectory, const wstring & directoryPath)
{
    std::shared_ptr<InstalledPackage> packageInfo;
    auto res = PopulatePackageInfo::GetPackageInfoFromManifest(directoryPath.c_str(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo);
    if (FAILED(res))
    {
        return nullptr;
    }
    return std::dynamic_pointer_cast<IInstalledPackage>(packageInfo);
}

shared_ptr<IInstalledPackage> PackageManager::FindPackage(const wstring & packageFullName)
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    wstring msixCoreDirectory = filemapping.GetMsixCoreDirectory();
    wstring packageDirectoryPath = msixCoreDirectory + packageFullName;
    auto package = GetPackageInfo(msixCoreDirectory, packageDirectoryPath);
    return package;
}

shared_ptr<IInstalledPackage> PackageManager::FindPackageByFamilyName(const wstring & packageFamilyName)
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    auto msixCoreDirectory = filemapping.GetMsixCoreDirectory();
    for (auto& p : experimental::filesystem::directory_iterator(msixCoreDirectory))
    {

        auto installedAppFamilyName = GetFamilyNameFromFullName(p.path().filename());
        if (CaseInsensitiveEquals(installedAppFamilyName, packageFamilyName))
        {
            return GetPackageInfo(msixCoreDirectory, p.path());
        }
    }
    return nullptr;
}

unique_ptr<vector<shared_ptr<IInstalledPackage>>> PackageManager::FindPackages()
{
    auto packages = std::make_unique<std::vector<shared_ptr<IInstalledPackage>>>();
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return packages;
    }
    auto msixCoreDirectory = filemapping.GetMsixCoreDirectory();
    for (auto& p : experimental::filesystem::directory_iterator(msixCoreDirectory))
    {
        auto packageInfo = GetPackageInfo(msixCoreDirectory, p.path());
        if (packageInfo != nullptr)
        {
            packages->push_back(packageInfo);
        }
    }
    return packages;
}

shared_ptr<IPackage> PackageManager::GetMsixPackageInfo(const wstring & msixFullPath)
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    shared_ptr<Package> packageInfo;
    res = PopulatePackageInfo::GetPackageInfoFromPackage(msixFullPath.c_str(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo);
    if (FAILED(res))
    {
        return nullptr;
    }
    return dynamic_pointer_cast<IPackage>(packageInfo);
}