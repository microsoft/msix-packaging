#include "PackageManager.hpp"
#include "MsixRequest.hpp"
#include "Constants.hpp"
#include "PopulatePackageInfo.hpp"
#include "MsixTraceLoggingProvider.hpp"
#include <experimental/filesystem>
#include <thread>
#include <regex>
#include "Windows10Redirector.hpp"
#include <string>

using namespace std;
using namespace MsixCoreLib;

PackageManager::PackageManager()
{
}

shared_ptr<IMsixResponse> PackageManager::AddPackageAsync(const wstring & packageFilePath, DeploymentOptions options, function<void(const IMsixResponse&)> callback)
{
    if (IsWindows10RS3OrLater())
    {
        auto msixResponse = std::make_shared<MsixResponse>();
        msixResponse->SetCallback(callback);

        auto t = thread([&](shared_ptr<MsixResponse> response) {
            Windows10Redirector::AddPackageWithProgress(packageFilePath, response);
        }, msixResponse);
        t.detach();

        return msixResponse;
    }

    ComPtr<IStream> packageStream;
    if (FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), /*forRead */ true, &packageStream)))
    {
        return nullptr;
    }

    return AddPackageAsync(packageStream.Get(), options, callback);
}

shared_ptr<IMsixResponse> PackageManager::AddPackageAsync(IStream * packageStream, DeploymentOptions options, function<void(const IMsixResponse&)> callback)
{
    if (IsWindows10RS3OrLater())
    {
        auto msixResponse = std::make_shared<MsixResponse>();
        msixResponse->SetCallback(callback);

        TCHAR tempPackagePath[MAX_PATH];
        if (FAILED(Windows10Redirector::ConvertIStreamToPackagePath(packageStream, tempPackagePath)))
        {
            return nullptr;
        }

        auto t = thread([&](shared_ptr<MsixResponse> response) {
            Windows10Redirector::AddPackageWithProgress(tempPackagePath, response);
        }, msixResponse);
        t.detach();

        return msixResponse;
    }

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

HRESULT PackageManager::AddPackage(const wstring & packageFilePath, DeploymentOptions options)
{
    if (IsWindows10RS3OrLater())
    {
        RETURN_IF_FAILED(Windows10Redirector::AddPackage(packageFilePath));
        return S_OK;
    }

    ComPtr<IStream> packageStream;
    auto res = CreateStreamOnFileUTF16(packageFilePath.c_str(), /*forRead */ true, &packageStream);
    if (FAILED(res))
    {
        return res;
    }

    return AddPackage(packageStream.Get(), options);
}

HRESULT PackageManager::AddPackage(IStream * packageStream, DeploymentOptions options)
{
    if (IsWindows10RS3OrLater())
    {
        TCHAR tempPackagePath[MAX_PATH];
        RETURN_IF_FAILED(Windows10Redirector::ConvertIStreamToPackagePath(packageStream, tempPackagePath));
        RETURN_IF_FAILED(Windows10Redirector::AddPackage(tempPackagePath));

        return S_OK;
    }

    AutoPtr<MsixRequest> impl;
    RETURN_IF_FAILED(MsixRequest::Make(OperationType::Add, packageStream, L"", MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &impl));
    
    RETURN_IF_FAILED(impl->ProcessRequest());
    return S_OK;
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
    if (IsWindows10RS3OrLater())
    {
        RETURN_IF_FAILED(Windows10Redirector::RemovePackage(packageFullName));
        return S_OK;
    }

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
    wstring packageFullNameCopy = packageFullName;

    packageFullNameCopy = std::regex_replace(packageFullNameCopy, std::wregex(L"\\*"), L".*");
    packageFullNameCopy = std::regex_replace(packageFullNameCopy, std::wregex(L"\\?"), L".");

    std::string packageFullNameString(packageFullNameCopy.begin(), packageFullNameCopy.end());
    std::regex packageFullNameRegExp(packageFullNameString);

    for (auto& p : experimental::filesystem::directory_iterator(msixCoreDirectory))
    {
        if (std::regex_match(p.path().filename().string(), packageFullNameRegExp))
        {
            wstring packageDirectoryPath = msixCoreDirectory + p.path().filename().c_str();
            RETURN_IF_FAILED(GetPackageInfo(packageDirectoryPath, installedPackage));
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT PackageManager::FindPackageByFamilyName(const wstring & packageFamilyName, shared_ptr<IInstalledPackage>& installedPackage)
{
    auto filemapping = FilePathMappings::GetInstance();
    RETURN_IF_FAILED(filemapping.GetInitializationResult());
    auto msixCoreDirectory = filemapping.GetMsixCoreDirectory();

    wstring packageFamilyNameCopy = packageFamilyName;

    packageFamilyNameCopy = std::regex_replace(packageFamilyNameCopy, std::wregex(L"\\*"), L".*");
    packageFamilyNameCopy = std::regex_replace(packageFamilyNameCopy, std::wregex(L"\\?"), L".");

    std::string packageFamilyNameString(packageFamilyNameCopy.begin(), packageFamilyNameCopy.end());
    std::regex packageFamilyNameRegExp(packageFamilyNameString);

    for (auto& p : experimental::filesystem::directory_iterator(msixCoreDirectory))
    {
        if (experimental::filesystem::is_directory(p.path()))
        {
            wstring installedAppFamilyName = GetFamilyNameFromFullName(p.path().filename());
            std::string installedAppFamilyNameString(installedAppFamilyName.begin(), installedAppFamilyName.end());

            if (std::regex_match(installedAppFamilyNameString, packageFamilyNameRegExp))
            {
                wstring packageDirectoryPath = msixCoreDirectory + std::wstring(p.path().filename());
                RETURN_IF_FAILED(GetPackageInfo(packageDirectoryPath, installedPackage));
                return S_OK;
            }
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
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