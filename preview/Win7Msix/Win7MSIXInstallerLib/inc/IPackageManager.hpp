#pragma once
#include <string>
#include <vector>
#include <functional>
#include "IMsixResponse.hpp"
#include "IPackage.hpp"
#include "DeploymentOptions.hpp"


namespace Win7MsixInstallerLib {
    class IPackageManager
    {
    public:
        virtual IMsixResponse * AddPackage(const std::wstring & packageFilePath, DeploymentOptions options) = 0;
        virtual IMsixResponse * RemovePackage(const std::wstring & packageFullName) = 0;
        virtual IInstalledPackageInfo * FindPackage(const std::wstring & packageFamilyName) = 0;
        virtual IInstalledPackageInfo * FindPackageByFamilyName(const std::wstring & packageFamilyName) = 0;
        virtual std::vector<IInstalledPackageInfo*> * FindPackages() = 0;
        virtual IPackage * GetPackageInfoMsix(const std::wstring & msixFullPath) = 0;
    };
}
