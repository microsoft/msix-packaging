#pragma once
#include <string>
#include <vector>
#include <functional>
#include "IMsixResponse.hpp"
#include "IPackage.hpp"
#include "DeploymentOptions.hpp"
#include "AppxPackaging.hpp"

namespace MsixCoreLib {
    class IPackageManager
    {
    public:
        virtual std::shared_ptr<IMsixResponse> AddPackageAsync(const std::wstring & packageFilePath, DeploymentOptions options, std::function<void(const IMsixResponse&)> callback = nullptr) = 0;
        virtual std::shared_ptr<IMsixResponse> AddPackageAsync(IStream * packageStream, DeploymentOptions options, std::function<void(const IMsixResponse&)> callback = nullptr) = 0;
        virtual HRESULT AddPackage(const std::wstring & packageFilePath, DeploymentOptions options) = 0;
        virtual HRESULT AddPackage(IStream * packageStream, DeploymentOptions options) = 0;
        virtual std::shared_ptr<IMsixResponse> RemovePackageAsync(const std::wstring & packageFullName, std::function<void(const IMsixResponse&)> callback = nullptr) = 0;
        virtual HRESULT RemovePackage(const std::wstring & packageFullName) = 0;
        virtual HRESULT FindPackage(const std::wstring & packageFullName, std::shared_ptr<IInstalledPackage> &package) = 0;
        virtual HRESULT FindPackageByFamilyName(const std::wstring & packageFamilyName, std::shared_ptr<IInstalledPackage> & installedPackage) = 0;
        virtual HRESULT FindPackages(const std::wstring & searchParameter, std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> & installedPackages) = 0;
        virtual HRESULT GetMsixPackageInfo(const std::wstring & msixFullPath, std::shared_ptr<IPackage> & package, MSIX_VALIDATION_OPTION validationOption) = 0;
    };
}
