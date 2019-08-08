#pragma once
#include "inc/IPackageManager.hpp"
#include "inc/IPackage.hpp"

namespace MsixCoreLib {

    class PackageManager :
        public IPackageManager
    {
    public:
        PackageManager();
        std::shared_ptr<IMsixResponse> AddPackageAsync(const std::wstring & packageFilePath, DeploymentOptions options, std::function<void(const IMsixResponse&)> callback = nullptr) override;
        std::shared_ptr<IMsixResponse> AddPackageAsync(IStream * packageStream, DeploymentOptions options, std::function<void(const IMsixResponse&)> callback = nullptr) override;
        HRESULT AddPackage(const std::wstring & packageFilePath, DeploymentOptions options) override;
        HRESULT AddPackage(IStream * packageStream, DeploymentOptions options) override;
        std::shared_ptr<IMsixResponse> RemovePackageAsync(const std::wstring & packageFullName, std::function<void(const IMsixResponse&)> callback = nullptr) override;
        HRESULT RemovePackage(const std::wstring & packageFullName) override;
        HRESULT FindPackage(const std::wstring & packageFullName, std::shared_ptr<IInstalledPackage> & installedPackage) override;
        HRESULT FindPackageByFamilyName(const std::wstring & packageFamilyName, std::shared_ptr<IInstalledPackage> & installedPackage) override;
        HRESULT FindPackages(const std::wstring & searchParameter, std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> & installedPackages) override;
        HRESULT GetMsixPackageInfo(const std::wstring & msixFullPath, std::shared_ptr<IPackage> & package) override;
    private:
        HRESULT GetPackageInfo(const std::wstring & directoryPath, std::shared_ptr<IInstalledPackage> & installedPackage);
        HRESULT CreateStreamOnPackageUrl(const std::wstring & package, IStream** stream);
    };
}