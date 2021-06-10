#pragma once

#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "DirectoryObject.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxPackageInfo.hpp"
#include "AppxManifestObject.hpp"
#include "VersionHelpers.hpp"

#include <string>
#include <map>
#include <algorithm>

namespace MSIX {

    struct PackageNameInfo
    {
        std::string packageFullName;
        std::string fileName;
    };

    class BundleValidationHelper
    {
    public:
        BundleValidationHelper();

        void ValidateTargetDeviceFamiliesFromManifestPackageId(IAppxManifestPackageIdInternal* packageId, IAppxManifestTargetDeviceFamiliesEnumerator* tdfs,
            std::string fileName);

        bool StartsWith(std::string str, std::string prefix);

        bool ContainsMultipleNeutralAppPackages();

        bool ContainsApplicationPackage();

        void ValidateContainsMultipleNeutralAppPackages(bool isPre2018BundleManifest);

        void AddPackage(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type, IAppxManifestPackageIdInternal* packageId, std::string fileName);

        void ValidateResourcePackage(IAppxManifestPackageIdInternal* packageId, PackageNameInfo packageNameInfo);

        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE GetPayloadPackageType(IAppxManifestReader* packageManifestReader, std::string fileName);

        void ValidateOSVersion(IAppxManifestReader* packageManifestReader, std::string fileName);

        void ValidateApplicationElement(IAppxManifestReader* packageManifestReader, std::string fileName);

    private:
        const std::string MinimumAllowedOSVersion = "6.3.0.0";
        std::uint32_t numberOfNeutralAppPackages = 0;
        // This map contains the set of all target device families from neutral application packages that have been added.
        std::map<std::string, PackageNameInfo> neutralTdfs;
        bool bundleTargetsRs5OrLess = false;
        std::uint32_t MaxNumberOfPackages = 10000;
        std::uint32_t numberOfPackagesAdded = 0;
        bool containsApplicationPackage = false;

        // This map contains the set of all package file names from resource packages that have been added.
        std::map<std::string, PackageNameInfo> fileNamesMap;

        // This map contains the set of all resource IDs from resource packages that have been added.
        std::map<std::string, PackageNameInfo> resourceIds;
    };
}