#include "BundleWriterHelper.hpp"

namespace MSIX {

    BundleWriterHelper::BundleWriterHelper() {}

    HRESULT BundleWriterHelper::GetStreamSize(_In_ IStream* stream, _Out_ UINT64* sizeOfStream)
    {
        HRESULT hr = S_OK;
        STATSTG stat;
        ThrowHrIfFailed(stream->Stat(&stat, STATFLAG_NONAME));

        *sizeOfStream = stat.cbSize.QuadPart;
        return S_OK;
    }

    HRESULT BundleWriterHelper::AddPackage(_In_ std::string fileName, _In_ IAppxPackageReader* packageReader,
        _In_ std::uint64_t bundleOffset, _In_ std::uint64_t packageSize, _In_ bool isDefaultApplicableResource)
    {
        ComPtr<IAppxManifestPackageId> packageId;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        //ComPtr<IAppxManifestQualifiedResourcesEnumerator> resources;
                ComPtr<IAppxManifestResourcesEnumerator> resources;

        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfs;

        ThrowHrIfFailed(GetValidatedPackageData(fileName, packageReader, &packageType, &packageId, &resources, &tdfs));

        ThrowHrIfFailed(AddValidatedPackageData(fileName, bundleOffset, packageSize, packageType, packageId,
                isDefaultApplicableResource, resources.Get(), tdfs.Get()));
        return S_OK;
    }

    HRESULT BundleWriterHelper::GetValidatedPackageData(
        _In_ std::string fileName,
        _In_ IAppxPackageReader* packageReader,
        _Out_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType,
        _Outptr_result_nullonfailure_ IAppxManifestPackageId** packageId,
        _Outptr_result_nullonfailure_ IAppxManifestResourcesEnumerator** resources,
        _Outptr_result_maybenull_ IAppxManifestTargetDeviceFamiliesEnumerator** tdfs)
    {
        *packageId = nullptr;
        *resources = nullptr;
        *tdfs = nullptr;

        ComPtr<IAppxManifestPackageId> loadedPackageId;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE loadedPackageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        //ComPtr<IAppxManifestQualifiedResourcesEnumerator> loadedResources;
        ComPtr<IAppxManifestResourcesEnumerator> loadedResources;
        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> loadedTdfs;

        ComPtr<IAppxManifestReader> manifestReader;
        ThrowHrIfFailed(packageReader->GetManifest(&manifestReader));
        ThrowHrIfFailed(manifestReader->GetPackageId(&loadedPackageId));

        ComPtr<IAppxManifestReader3> manifestReader3;
        ThrowHrIfFailed(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));

        ThrowHrIfFailed(manifestReader->GetResources(&loadedResources));

        HRESULT hr = manifestReader3->GetTargetDeviceFamilies(&loadedTdfs);
        if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
        {
            return hr;
        }

        ThrowHrIfFailed(GetPayloadPackageType(manifestReader.Get(), fileName, &loadedPackageType));
        //AddPackage checks
        //ValidateOSVersion checks
        //ThrowHrIfFailed(PackageMatchesHashMethod(packageReader, fileName));

        auto packageIdInternal = loadedPackageId.As<IAppxManifestPackageIdInternal>();
        ThrowHrIfFailed(ValidateNameAndPublisher(packageIdInternal.Get(), fileName));

        //TDF check

        if (loadedPackageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            ThrowHrIfFailed(ValidateApplicationElement(manifestReader.Get(), fileName));

            /*if (loadedTdfs != nullptr)
            {
                ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfCopy;
                ThrowHrIfFailed(manifestReader3->GetTargetDeviceFamilies(&tdfCopy));
                //TDF Checks
            }*/
        }

        *packageType = loadedPackageType;
        *packageId = loadedPackageId.Detach();
        *resources = loadedResources.Detach();

        if (loadedTdfs.Get() != nullptr)
        {
            *tdfs = loadedTdfs.Detach();
        }
         return S_OK;
    }

    HRESULT BundleWriterHelper::ValidateApplicationElement(
        _In_ IAppxManifestReader* packageManifestReader,
        _In_ std::string fileName)
    {
        HRESULT hr = S_OK;
        ComPtr<IAppxManifestReader4> manifestReader4;
        ThrowHrIfFailed(packageManifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader4>::iid, reinterpret_cast<void**>(&manifestReader4)));

        ComPtr<IAppxManifestOptionalPackageInfo> optionalPackageInfo;
        ThrowHrIfFailed(manifestReader4->GetOptionalPackageInfo(&optionalPackageInfo));
        
        BOOL packageIsOptional = FALSE;
        ThrowHrIfFailed(optionalPackageInfo->GetIsOptionalPackage(&packageIsOptional));

        if (!packageIsOptional) // optional payload packages are not required to declare any <Application> elements
        {
            ComPtr<IAppxManifestApplicationsEnumerator> applications;
            ThrowHrIfFailed(packageManifestReader->GetApplications(&applications));
            BOOL hasApplication = FALSE;
            ThrowHrIfFailed(applications->GetHasCurrent(&hasApplication));

            if (!hasApplication)
            {
                //LPWSTR packageFullName;
                //GetPackageFullNameFromManifest(packageManifestReader, packageFullName);
                //Log error NO_APPLICATION, fileName, packageFullName
                //return APPX_E_INVALID_MANIFEST;
            }
        }
        return hr;
    }

    HRESULT BundleWriterHelper::ValidateNameAndPublisher(
        _In_ IAppxManifestPackageIdInternal* packageId,
        _In_ std::string filename)
    {
        if(this->mainPackageName.empty())
        {
            this->mainPackageName = packageId->GetName();
            this->mainPackagePublisher = packageId->GetPublisher();
        }
        else
        {
            std::string packageName = packageId->GetName();
            
            if ((this->mainPackageName.compare(packageName)) != 0)
            {
                std::string packageFullName = packageId->GetPackageFullName();
                //Log mismatched packagename error, filename, packageFullName.get(), this->mainPackageName
                //return APPX_E_INVALID_MANIFEST;
            }

            std::string publisherName = packageId->GetPublisher();
            if ((this->mainPackagePublisher.compare(publisherName)) != 0)
            {
                std::string packageFullName = packageId->GetPackageFullName();
                //Log mismatched publisher error, filename, packageFullName.get(), this->mainPackagePublisher
                //return APPX_E_INVALID_MANIFEST;
            }
        }
        return S_OK;
    }

    HRESULT BundleWriterHelper::PackageMatchesHashMethod(
        _In_ IAppxPackageReader* packageReader,
        _In_ std::string fileName)
    {
        HRESULT hr = S_OK;
        ComPtr<IAppxBlockMapReader> blockMapReader;
        ThrowHrIfFailed(packageReader->GetBlockMap(&blockMapReader));

        //ComPtr<IUri> hashMethod;
        //ThrowHrIfFailed(blockMapReader->GetHashMethod(&hashMethod));

        //BSTR hashAlgorithmUri;
        //ThrowHrIfFailed(hashMethod->GetAbsoluteUri(&hashAlgorithmUri));

        /*ComPtr<IUri> expectedHashMethod;
        std::wstring hashMethodString = L"http://www.w3.org/2001/04/xmlenc#sha256";
        ThrowHrIfFailed(CreateUri(hashMethodString, 0x0001, NULL, &hashMethod));*/

        //std::wstring hashAlgorithmUri;
        //ThrowHrIfFailed(hashMethod->GetAbsoluteUri(&hashAlgorithmUri));

        /*if(!(wcscmp(hashAlgorithmUri, L"http://www.w3.org/2001/04/xmlenc#sha256") == 0))
        {
            //return APPX_E_INVALID_BLOCKMAP;
        }*/

        return hr;
    }

    HRESULT BundleWriterHelper::GetPayloadPackageType(
        _In_ IAppxManifestReader* packageManifestReader,
        _In_ std::string fileName,
        _Out_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType)
    {
        HRESULT hr = S_OK;
        ComPtr<IAppxManifestProperties> packageProperties;
        ThrowHrIfFailed(packageManifestReader->GetProperties(&packageProperties));

        BOOL isFrameworkPackage = FALSE;
        hr = packageProperties->GetBoolValue(L"Framework", &isFrameworkPackage);
        if (FAILED(hr) && (hr != E_INVALIDARG))
        {
            return hr;
        }

        if (isFrameworkPackage)
        {
            //This method will fail with
            /// APPX_E_INVALID_MANIFEST if the manifest is for a Framework package.
            //return APPX_E_INVALID_MANIFEST;
        }

        BOOL isResourcePackage = FALSE;
        hr = packageProperties->GetBoolValue(L"ResourcePackage", &isResourcePackage);
        if (FAILED(hr) && (hr != E_INVALIDARG))
        {
            return hr;
        }

        *packageType = (isResourcePackage ? APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE : APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION);
        return S_OK;
    }

    HRESULT BundleWriterHelper::AddValidatedPackageData(
        _In_ std::string fileName,
        _In_ std::uint64_t bundleOffset,
        _In_ std::uint64_t packageSize,
        _In_ APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType,
        _In_ ComPtr<IAppxManifestPackageId> packageId,
        _In_ bool isDefaultApplicablePackage,
        _In_ IAppxManifestResourcesEnumerator* resources,
        _In_ IAppxManifestTargetDeviceFamiliesEnumerator* tdfs)
    {
        //validate package payload extension

        auto innerPackageIdInternal = packageId.As<IAppxManifestPackageIdInternal>();

        PackageInfo packageInfo;
        packageInfo.type = packageType;
        packageInfo.architecture = innerPackageIdInternal->GetArchitecture();
        ThrowHrIfFailed(packageId->GetVersion(&packageInfo.version));
        packageInfo.resourceId = innerPackageIdInternal->GetResourceId();
        packageInfo.isDefaultApplicablePackage = isDefaultApplicablePackage;
        packageInfo.resources = resources;
        packageInfo.fileName = fileName;
        packageInfo.size = packageSize;
        packageInfo.offset = bundleOffset;
        packageInfo.tdfs = tdfs;

        ThrowHrIfFailed(AddPackageInfoToVector(this->payloadPackages, packageInfo));

        return S_OK;
    }

    HRESULT BundleWriterHelper::AddPackageInfoToVector(std::vector<PackageInfo>& packagesVector, 
        PackageInfo packageInfo)
    {
        packagesVector.push_back(packageInfo);

        if (packageInfo.offset == 0)
        {
            this->hasExternalPackages = true;
        }

        if (packageInfo.isDefaultApplicablePackage)
        {
            this->hasDefaultOrNeutralResources = true;
        }

        BOOL hasResources = FALSE;
        ThrowHrIfFailed(packageInfo.resources->GetHasCurrent(&hasResources));
        if (!hasResources)
        {
            this->hasDefaultOrNeutralResources = true;
        }

        return S_OK;
    }

    HRESULT BundleWriterHelper::EndBundleManifest()
    {
        std::string targetXmlNamespace = "http://schemas.microsoft.com/appx/2013/bundle";
        //Compute and assign Namespace for neutral resources or optional bundles

        m_bundleManifestWriter.StartBundleManifest(targetXmlNamespace, this->mainPackageName,
            this->mainPackagePublisher, this->bundleVersion);

        for(std::size_t i = 0; i < this->payloadPackages.size(); i++) 
        {
            m_bundleManifestWriter.WritePackageElement(payloadPackages[i]);
        }

        //Do the same loop for this->OptionalBundles
        
        //Ends Packages and bundle Element
        m_bundleManifestWriter.EndPackagesElement();
        m_bundleManifestWriter.Close();
        return S_OK;
    }

}