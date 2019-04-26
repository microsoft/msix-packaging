#include <windows.h>

#include "PopulatePackageInfo.hpp"
#include "GeneralUtil.hpp"
#include "FilePaths.hpp"
#include <TraceLoggingProvider.h>
#include <experimental/filesystem> // C++-standard header file name
#include "Constants.hpp"
#include "MsixTraceLoggingProvider.hpp"
using namespace MsixCoreLib;

const PCWSTR PopulatePackageInfo::HandlerName = L"PopulatePackageInfo";

HRESULT PopulatePackageInfo::GetPackageInfoFromPackage(const std::wstring & packageFilePath, MSIX_VALIDATION_OPTION validationOption, std::shared_ptr<Package> * packageInfo)
{
    ComPtr<IStream> inputStream;
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(packageFilePath.c_str(), /*forRead */ true, &inputStream));

    // On Win32 platforms CoCreateAppxFactory defaults to CoTaskMemAlloc/CoTaskMemFree
    // On non-Win32 platforms CoCreateAppxFactory will return 0x80070032 (e.g. HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
    // So on all platforms, it's always safe to call CoCreateAppxFactoryWithHeap, just be sure to bring your own heap!
    ComPtr<IAppxFactory> appxFactory;
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, validationOption, &appxFactory));

    // Create a new package reader using the factory.
    ComPtr<IAppxPackageReader> packageReader;
    RETURN_IF_FAILED(appxFactory->CreatePackageReader(inputStream.Get(), &packageReader));
    RETURN_IF_FAILED(Package::MakeFromPackageReader(packageReader.Get(), packageInfo));
    packageReader.Release();
    inputStream.Release();

    return S_OK;
}

HRESULT PopulatePackageInfo::GetPackageInfoFromManifest(const std::wstring & directoryPath, MSIX_VALIDATION_OPTION validationOption, std::shared_ptr<InstalledPackage> * packageInfo)
{
    std::wstring manifestPath = directoryPath + manifestFile;

    // On Win32 platforms CoCreateAppxFactory defaults to CoTaskMemAlloc/CoTaskMemFree
    // On non-Win32 platforms CoCreateAppxFactory will return 0x80070032 (e.g. HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
    // So on all platforms, it's always safe to call CoCreateAppxFactoryWithHeap, just be sure to bring your own heap!
    ComPtr<IStream> inputStream;
    RETURN_IF_FAILED(CreateStreamOnFileUTF16(manifestPath.c_str(), /*forRead*/ true, &inputStream));

    // Create a new package reader using the factory.
    ComPtr<IAppxFactory> appxFactory;
    RETURN_IF_FAILED(CoCreateAppxFactoryWithHeap(MyAllocate, MyFree, validationOption, &appxFactory));

    ComPtr<IAppxManifestReader> manifestReader;
    RETURN_IF_FAILED(appxFactory->CreateManifestReader(inputStream.Get(), &manifestReader));

    RETURN_IF_FAILED(InstalledPackage::MakeFromManifestReader(directoryPath, manifestReader.Get(), packageInfo));
    manifestReader.Release();
    inputStream.Release();
    return S_OK;
}


HRESULT PopulatePackageInfo::ExecuteForAddRequest()
{

    std::shared_ptr<Package> packageInfo;
    RETURN_IF_FAILED(PopulatePackageInfo::GetPackageInfoFromPackage(m_msixRequest->GetPackageFilePath(), m_msixRequest->GetValidationOptions(), &packageInfo));

    if (packageInfo == nullptr)
    {
        return E_FAIL;
    }

    m_msixRequest->SetPackageInfo(packageInfo);

    TraceLoggingWrite(g_MsixTraceLoggingProvider,
        "PackageInfo",
        TraceLoggingValue(packageInfo->GetPackageFullName().c_str(), "PackageFullName"),
        TraceLoggingValue(packageInfo->GetNumberOfPayloadFiles(), "NumberOfPayloadFiles"),
        TraceLoggingValue(m_msixRequest->GetPackageDirectoryPath().c_str(), "ExecutableFilePath"),
        TraceLoggingValue(packageInfo->GetDisplayName().c_str(), "DisplayName"));


    return S_OK;
}

HRESULT PopulatePackageInfo::ExecuteForRemoveRequest()
{
    auto packageDirectoryPath = FilePathMappings::GetInstance().GetMsix7Directory() + m_msixRequest->GetPackageFullName();

    std::shared_ptr<InstalledPackage> package;
    RETURN_IF_FAILED(GetPackageInfoFromManifest(packageDirectoryPath, MSIX_VALIDATION_OPTION::MSIX_VALIDATION_OPTION_ALLOWSIGNATUREORIGINUNKNOWN, &package));

    if (package == nullptr)
    {
        return E_FAIL;
    }
    m_msixRequest->SetPackageInfo(package);

    return S_OK;
}

HRESULT PopulatePackageInfo::CreateHandler(MsixRequest * msixRequest, IPackageHandler ** instance)
{
    std::unique_ptr<PopulatePackageInfo> localInstance(new PopulatePackageInfo(msixRequest));
    if (localInstance == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    *instance = localInstance.release();

    return S_OK;
}