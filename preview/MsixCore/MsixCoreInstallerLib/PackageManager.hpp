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
        HRESULT AddPackage(const std::wstring & packageFilePath, DeploymentOptions options) override;
        std::shared_ptr<IMsixResponse> RemovePackageAsync(const std::wstring & packageFullName, std::function<void(const IMsixResponse&)> callback = nullptr) override;
        HRESULT RemovePackage(const std::wstring & packageFullName) override;
        std::shared_ptr<IInstalledPackage> FindPackage(const std::wstring & packageFullName) override;
        std::shared_ptr<IInstalledPackage> FindPackageByFamilyName(const std::wstring & packageFamilyName) override;
        std::unique_ptr<std::vector<std::shared_ptr<IInstalledPackage>>> FindPackages() override;
        std::shared_ptr<IPackage> GetMsixPackageInfo(const std::wstring & msixFullPath) override;
    private:
        std::shared_ptr<IInstalledPackage> GetPackageInfo(const std::wstring & msixCoreDirectory, const std::wstring & directoryPath);
    };
}