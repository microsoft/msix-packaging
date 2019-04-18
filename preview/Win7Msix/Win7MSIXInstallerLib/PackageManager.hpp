#pragma once
#include "inc/IPackageManager.hpp"
#include "inc/IPackage.hpp"

namespace Win7MsixInstallerLib {

    class PackageManager :
        public IPackageManager
    {
    public:
        PackageManager();
        IMsixResponse * AddPackage(const std::wstring & packageFilePath, DeploymentOptions options) override;
        IMsixResponse * RemovePackage(const std::wstring & packageFullName) override;
        IInstalledPackageInfo * FindPackage(const std::wstring & packageFullName) override;
        IInstalledPackageInfo * FindPackageByFamilyName(const std::wstring & packageFamilyName) override;
        std::vector<IInstalledPackageInfo *> * FindPackages() override;
        IPackage * GetPackageInfoMsix(const std::wstring & msixFullPath) override;
    private:
        IInstalledPackageInfo * GetPackageInfo(const std::wstring & msix7Directory, const std::wstring & directoryPath);
    };
}