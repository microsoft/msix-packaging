#include "PackageManager.hpp"
#include "MsixRequest.hpp"
#include "Constants.hpp"
#include "PopulatePackageInfo.hpp"
#include "MsixTraceLoggingProvider.hpp"
#include <filesystem>
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
    if (FAILED(CreateStreamOnPackageUrl(packageFilePath.c_str(), &packageStream)))
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

        WCHAR tempPackagePath[MAX_PATH];
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
    RETURN_IF_FAILED(CreateStreamOnPackageUrl(packageFilePath, &packageStream));
    
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

HRESULT MsixCoreLib::PackageManager::CreateStreamOnPackageUrl(const std::wstring & package, IStream ** stream)
{
    std::wstring httpPrefix(L"http");
    bool isPathHttp = package.compare(0, httpPrefix.length(), httpPrefix) == 0;
    if (isPathHttp)
    {
        TraceLoggingWrite(g_MsixTraceLoggingProvider,
            "HTTP file detected, will download to cache",
            TraceLoggingValue(package.c_str(), "PackageFilePath"));
        WCHAR fileName[MAX_PATH];
        RETURN_IF_FAILED(URLDownloadToCacheFile(nullptr /*activex iunknown*/, package.c_str(), fileName, MAX_PATH, 0 /*reserved*/, nullptr /*bindstatuscallback*/));

        RETURN_IF_FAILED(CreateStreamOnFileUTF16(fileName, true /*forRead*/, stream));
    }
    else // file path
    {
        RETURN_IF_FAILED(CreateStreamOnFileUTF16(package.c_str(), true /*forRead */, stream));
    }

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

    for (auto& p : filesystem::directory_iterator(msixCoreDirectory))
    {
        if (filesystem::is_directory(p.path()))
        {
            auto installedAppFamilyName = GetFamilyNameFromFullName(p.path().filename());
            if (CaseInsensitiveEquals(installedAppFamilyName, packageFamilyName))
            {
                wstring packageDirectoryPath = msixCoreDirectory + std::wstring(p.path().filename());
                RETURN_IF_FAILED(GetPackageInfo(packageDirectoryPath, installedPackage));
                return S_OK;
            }
        }
    }
    return S_OK;
}

HRESULT PackageManager::FindPackages(const std::wstring & searchParameter, unique_ptr<vector<shared_ptr<IInstalledPackage>>> & installedPackages)
{
    auto packages = std::make_unique<std::vector<shared_ptr<IInstalledPackage>>>();

    auto filemapping = FilePathMappings::GetInstance();
    RETURN_IF_FAILED(filemapping.GetInitializationResult());
    wstring msixCoreDirectory = filemapping.GetMsixCoreDirectory();

    wstring searchParameterCopy = searchParameter;

    searchParameterCopy = std::regex_replace(searchParameterCopy, std::wregex(L"\\*"), L".*");
    searchParameterCopy = std::regex_replace(searchParameterCopy, std::wregex(L"\\?"), L".");

    std::string searchParameterString(searchParameterCopy.begin(), searchParameterCopy.end());
    std::regex searchParameterRegExp(searchParameterString, std::regex_constants::icase);

    std::vector<std::wstring> packageFullNames;
    for (auto& p : filesystem::directory_iterator(msixCoreDirectory))
    {
        if (filesystem::is_directory(p.path()))
        {
            wstring installedAppFamilyName = GetFamilyNameFromFullName(p.path().filename());
            std::string installedAppFamilyNameString(installedAppFamilyName.begin(), installedAppFamilyName.end());

            if ((std::regex_match(p.path().filename().string(), searchParameterRegExp)
                || std::regex_match(installedAppFamilyNameString, searchParameterRegExp)
                || CaseInsensitiveEquals(searchParameter, L"*")))
            {
                wstring packageDirectoryPath = msixCoreDirectory + std::wstring(p.path().filename());
                shared_ptr<IInstalledPackage> packageInfo;
                const HRESULT hrGetPackageInfo = GetPackageInfo(packageDirectoryPath, packageInfo);
                if (FAILED(hrGetPackageInfo))
                {
                    TraceLoggingWrite(g_MsixTraceLoggingProvider,
                        "Error getting package info from directory",
                        TraceLoggingValue(packageDirectoryPath.c_str(), "Directory"),
                        TraceLoggingValue(hrGetPackageInfo, "HR"));
                }
                else
                {
                    packages->push_back(packageInfo);
                }
            }
        }
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
    RETURN_IF_FAILED(CreateStreamOnPackageUrl(msixFullPath.c_str(), &packageStream));
    RETURN_IF_FAILED(PopulatePackageInfo::GetPackageInfoFromPackage(packageStream.Get(), MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_FULL, &packageInfo));
    
    package = dynamic_pointer_cast<IPackage>(packageInfo);
    return S_OK;
}