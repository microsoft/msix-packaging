#include "PackageManager.hpp"
#include "MsixRequest.hpp"
#include "Constants.hpp"
#include "PopulatePackageInfo.hpp"
#include <experimental/filesystem>
#include <thread>

using namespace std;
using namespace Win7MsixInstallerLib;

PackageManager::PackageManager()
{
}

IMsixResponse * PackageManager::AddPackageAsync(const wstring & packageFilePath, DeploymentOptions options)
{
    MsixRequest * impl;
    auto res = (MsixRequest::Make(OperationType::Add, packageFilePath, L"", MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(res))
    {
        return nullptr;
    }
    auto t = std::thread([&impl]() {
        impl->ProcessRequest();
        delete impl;
        impl = nullptr;
    });
    t.detach();
    return (IMsixResponse*)impl->GetMsixResponse();
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

IMsixResponse * PackageManager::RemovePackageAsync(const wstring & packageFullName)
{
    MsixRequest* impl;
    auto res = (MsixRequest::Make(OperationType::Remove, L"", packageFullName, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    if (FAILED(res))
    {
        return nullptr;
    }
    std::thread t([&impl]() {
        impl->ProcessRequest();
        impl = nullptr;
    });
    t.detach();
    return (IMsixResponse*)impl->GetMsixResponse();
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
IInstalledPackageInfo * PackageManager::GetPackageInfo(const std::wstring & msix7Directory, const std::wstring & directoryPath)
{
    InstalledPackage* packageInfo;
    auto res = PopulatePackageInfo::GetPackageInfoFromManifest(directoryPath.c_str(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo);
    if (FAILED(res))
    {
        return nullptr;
    }
    // we don't need to keep a reference to the manifest
    packageInfo->ReleaseManifest();
    return (IInstalledPackageInfo *)packageInfo;
}

IInstalledPackageInfo * PackageManager::FindPackage(const wstring & packageFullName)
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    std::wstring msix7Directory = filemapping.GetMsix7Directory();
    std::wstring packageDirectoryPath = msix7Directory + packageFullName;
    auto package = GetPackageInfo(msix7Directory, packageDirectoryPath);
    return package;
}

IInstalledPackageInfo * PackageManager::FindPackageByFamilyName(const std::wstring & packageFamilyName)
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    auto msix7Directory = filemapping.GetMsix7Directory();
    for (auto& p : std::experimental::filesystem::directory_iterator(msix7Directory))
    {

        auto installedAppFamilyName = Win7MsixInstallerLib_GetFamilyNameFromFullName(p.path().filename());
        if (Win7MsixInstallerLib_CaseInsensitiveEquals(installedAppFamilyName, packageFamilyName))
        {
            return GetPackageInfo(msix7Directory, p.path());
        }
    }
    return nullptr;
}

vector<IInstalledPackageInfo *> * PackageManager::FindPackages()
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    auto packages = new std::vector<IInstalledPackageInfo *>();
    auto msix7Directory = filemapping.GetMsix7Directory();
    for (auto& p : std::experimental::filesystem::directory_iterator(msix7Directory))
    {
        auto packageInfo = GetPackageInfo(msix7Directory, p.path());
        if (packageInfo != nullptr)
        {
            packages->push_back((IInstalledPackageInfo *)packageInfo);
        }
    }

    return packages;
}

IPackage * PackageManager::GetPackageInfoMsix(const wstring & msixFullPath)
{
    auto filemapping = FilePathMappings::GetInstance();
    auto res = filemapping.GetInitializationResult();
    if (FAILED(res))
    {
        return nullptr;
    }
    Package* packageInfo;
    res = PopulatePackageInfo::GetPackageInfoFromPackage(msixFullPath.c_str(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo);
    if (FAILED(res))
    {
        return nullptr;
    }
    return (IPackage *)packageInfo;
}