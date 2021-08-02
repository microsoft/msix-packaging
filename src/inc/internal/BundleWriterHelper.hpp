#pragma once

#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "DirectoryObject.hpp"
#include "AppxBlockMapWriter.hpp"
#include "ContentTypeWriter.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxPackageInfo.hpp"
#include "BundleManifestWriter.hpp"
#include "AppxManifestObject.hpp"
#include "BundleValidationHelper.hpp"

#include <map>

namespace MSIX {

    class BundleWriterHelper
    {
    public:
        BundleWriterHelper();

        std::uint64_t GetStreamSize(IStream* stream);

        void AddPackage(std::string fileName, IAppxPackageReader* packageReader, std::uint64_t bundleOffset,
            std::uint64_t packageSize, bool isDefaultApplicableResource);

        void GetValidatedPackageData(
            std::string fileName,
            IAppxPackageReader* packageReader,
            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType,
            IAppxManifestPackageId** packageId,
            IAppxManifestQualifiedResourcesEnumerator** resources,
            IAppxManifestTargetDeviceFamiliesEnumerator** tdfs);

        void AddValidatedPackageData(
            std::string fileName,
            std::uint64_t bundleOffset,
            std::uint64_t packageSize,
            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType,
            ComPtr<IAppxManifestPackageId> packageId,
            bool isDefaultApplicablePackage,
            IAppxManifestQualifiedResourcesEnumerator* resources,
            IAppxManifestTargetDeviceFamiliesEnumerator* tdfs);

        void ValidateNameAndPublisher(IAppxManifestPackageIdInternal* packageId, std::string filename);

        void AddPackageInfoToVector(std::vector<PackageInfo>& packagesVector, PackageInfo packageInfo);

        void EndBundleManifest();

        ComPtr<IStream> GetBundleManifestStream() { return m_bundleManifestWriter.GetStream(); }

        void SetBundleVersion(std::uint64_t bundleVersion) { this->bundleVersion = bundleVersion; }

        std::uint64_t GetBundleVersion() { return this->bundleVersion; }

        std::vector<PackageInfo> GetPayloadPackages() { return payloadPackages; }

        //Add External packages
        void AddExternalPackageReferenceFromManifest(std::string fileName, IAppxManifestReader* manifestReader,
            bool isDefaultApplicablePackage);

        std::uint64_t GetMinTargetDeviceFamilyVersionFromManifestForWindows(IAppxManifestReader* packageManifestReader);
    
    private:
        std::vector<PackageInfo> payloadPackages;
        std::map<std::string, OptionalBundleInfo> optionalBundles;
        bool hasExternalPackages;
        bool hasDefaultOrNeutralResources;
        std::string mainPackageName;
        std::string mainPackagePublisher;
        std::uint64_t bundleVersion;

        BundleValidationHelper m_validationHelper;
        BundleManifestWriter m_bundleManifestWriter;
    };
}