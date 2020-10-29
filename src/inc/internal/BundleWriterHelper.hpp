#pragma once

#include "AppxPackaging.hpp"
#include "ComHelper.hpp"
#include "DirectoryObject.hpp"
#include "AppxBlockMapWriter.hpp"
#include "ContentTypeWriter.hpp"
#include "ZipObjectWriter.hpp"
#include "BundleManifestWriter.hpp"
#include "AppxPackageInfo.hpp"

#include <map>
#include <memory>
#include <future>

namespace MSIX {

    struct PackageInfo
        {
            APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
            UINT64 version;
            std::string architecture;
            std::string resourceId;
            PCWSTR fileName;
            UINT64 size;
            UINT64 offset;
            //ComPtr<IAppxManifestQualifiedResourcesEnumerator> resources;
            ComPtr<IAppxManifestResourcesEnumerator> resources;

            BOOL isDefaultApplicablePackage;
            ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfs;
        };

    class BundleWriterHelper
    {
    public:
        BundleWriterHelper();

        HRESULT AddPackageInfoToVector(_In_ PackageInfo packageInfo);

        HRESULT GetStreamSize(_In_ IStream* stream, _Out_ UINT64* sizeOfStream);

        HRESULT AddPackage(_In_ PCWSTR fileName, _In_ IAppxPackageReader* packageReader, _In_ UINT64 bundleOffset,
            _In_ UINT64 packageSize, _In_ bool isDefaultApplicableResource);

        HRESULT GetValidatedPackageData(
            _In_ PCWSTR fileName,
            _In_ IAppxPackageReader* packageReader,
            _Out_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType,
            _Outptr_result_nullonfailure_ IAppxManifestPackageId** packageId,
            //_Outptr_result_nullonfailure_ IAppxManifestQualifiedResourcesEnumerator** resources,
                        _Outptr_result_nullonfailure_ IAppxManifestResourcesEnumerator** resources,

            _Outptr_result_maybenull_ IAppxManifestTargetDeviceFamiliesEnumerator** tdfs);

        HRESULT AddValidatedPackageData(
            _In_ PCWSTR fileName,
            _In_ UINT64 bundleOffset,
            _In_ UINT64 packageSize,
            _In_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType,
            _In_ ComPtr<IAppxManifestPackageId> packageId,
            _In_ BOOL isDefaultApplicablePackage,
            //_In_ IAppxManifestQualifiedResourcesEnumerator* resources,
            _In_ IAppxManifestResourcesEnumerator* resources,
            _In_ IAppxManifestTargetDeviceFamiliesEnumerator* tdfs);

        HRESULT GetPayloadPackageType(
            _In_ IAppxManifestReader* packageManifestReader,
            _In_ LPCWSTR fileName,
            _Out_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType);

        HRESULT PackageMatchesHashMethod(
            _In_ IAppxPackageReader* packageReader,
            _In_ LPCWSTR fileName);

        HRESULT ValidateNameAndPublisher(
            _In_ IAppxManifestPackageIdInternal* packageId,
            _In_ PCWSTR filename);

        HRESULT ValidateApplicationElement(
            _In_ IAppxManifestReader* packageManifestReader,
            _In_ LPCWSTR fileName);

        std::vector<PackageInfo> GetPayloadPackages();
    
    private:
        std::vector<PackageInfo> payloadPackages;
        bool hasExternalPackages;
        bool hasDefaultOrNeutralResources;
        std::string mainPackageName;
        std::string mainPackagePublisher;
        UINT64 bundleVersion;

    };
}