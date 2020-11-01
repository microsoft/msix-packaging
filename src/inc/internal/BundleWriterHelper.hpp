#pragma once

#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "DirectoryObject.hpp"
#include "AppxBlockMapWriter.hpp"
#include "ContentTypeWriter.hpp"
#include "ZipObjectWriter.hpp"
#include "AppxPackageInfo.hpp"
#include "BundleManifestWriter.hpp"

#include <map>
#include <memory>
#include <future>

namespace MSIX {

    class BundleWriterHelper
    {
    public:
        BundleWriterHelper();

        HRESULT AddPackageInfoToVector(std::vector<PackageInfo>& packagesVector, PackageInfo packageInfo);

        HRESULT GetStreamSize(_In_ IStream* stream, _Out_ UINT64* sizeOfStream);

        HRESULT AddPackage(_In_ std::string fileName, _In_ IAppxPackageReader* packageReader, _In_ std::uint64_t bundleOffset,
            _In_ std::uint64_t packageSize, _In_ bool isDefaultApplicableResource);

        HRESULT GetValidatedPackageData(
            _In_ std::string fileName,
            _In_ IAppxPackageReader* packageReader,
            _Out_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType,
            _Outptr_result_nullonfailure_ IAppxManifestPackageId** packageId,
            //_Outptr_result_nullonfailure_ IAppxManifestQualifiedResourcesEnumerator** resources,
                        _Outptr_result_nullonfailure_ IAppxManifestResourcesEnumerator** resources,

            _Outptr_result_maybenull_ IAppxManifestTargetDeviceFamiliesEnumerator** tdfs);

        HRESULT AddValidatedPackageData(
            _In_ std::string fileName,
            _In_ std::uint64_t bundleOffset,
            _In_ std::uint64_t packageSize,
            _In_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType,
            _In_ ComPtr<IAppxManifestPackageId> packageId,
            _In_ bool isDefaultApplicablePackage,
            //_In_ IAppxManifestQualifiedResourcesEnumerator* resources,
            _In_ IAppxManifestResourcesEnumerator* resources,
            _In_ IAppxManifestTargetDeviceFamiliesEnumerator* tdfs);

        HRESULT GetPayloadPackageType(
            _In_ IAppxManifestReader* packageManifestReader,
            _In_ std::string fileName,
            _Out_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType);

        HRESULT PackageMatchesHashMethod(
            _In_ IAppxPackageReader* packageReader,
            _In_ std::string fileName);

        HRESULT ValidateNameAndPublisher(
            _In_ IAppxManifestPackageIdInternal* packageId,
            _In_ std::string filename);

        HRESULT ValidateApplicationElement(
            _In_ IAppxManifestReader* packageManifestReader,
            _In_ std::string fileName);

        std::vector<PackageInfo> GetPayloadPackages() { return payloadPackages; }

        HRESULT EndBundleManifest();

        ComPtr<IStream> GetBundleManifestStream() { return m_bundleManifestWriter.GetStream(); }

        void SetBundleVersion(std::uint64_t bundleVersion) { this->bundleVersion = bundleVersion; }

        std::uint64_t GetBundleVersion() { return this->bundleVersion; }
    
    private:
        std::vector<PackageInfo> payloadPackages;
        bool hasExternalPackages;
        bool hasDefaultOrNeutralResources;
        std::string mainPackageName;
        std::string mainPackagePublisher;
        std::uint64_t bundleVersion;

        BundleManifestWriter m_bundleManifestWriter;

    };
}