#include "BundleWriterHelper.hpp"

namespace MSIX {

    BundleWriterHelper::BundleWriterHelper() 
    {
        this->bundleVersion = 0;
        this->hasExternalPackages = false;
        this->hasDefaultOrNeutralResources = false;
    }

    std::uint64_t BundleWriterHelper::GetStreamSize(IStream* stream)
    {
        STATSTG stat;
        ThrowHrIfFailed(stream->Stat(&stat, 1));

        return stat.cbSize.QuadPart;
    }

    void BundleWriterHelper::AddPackage(std::string fileName, IAppxPackageReader* packageReader,
        std::uint64_t bundleOffset, std::uint64_t packageSize, bool isDefaultApplicableResource)
    {
        ComPtr<IAppxManifestPackageId> packageId;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        ComPtr<IAppxManifestQualifiedResourcesEnumerator> resources;
        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfs;

        GetValidatedPackageData(fileName, packageReader, &packageType, &packageId, &resources, &tdfs);

        AddValidatedPackageData(fileName, bundleOffset, packageSize, packageType, packageId,
                isDefaultApplicableResource, resources.Get(), tdfs.Get());
    }

    void BundleWriterHelper::GetValidatedPackageData(
        std::string fileName,
        IAppxPackageReader* packageReader,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE* packageType,
        IAppxManifestPackageId** packageId,
        IAppxManifestQualifiedResourcesEnumerator** resources,
        IAppxManifestTargetDeviceFamiliesEnumerator** tdfs)
    {
        *packageId = nullptr;
        *resources = nullptr;
        *tdfs = nullptr;

        ComPtr<IAppxManifestPackageId> loadedPackageId;
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE loadedPackageType = APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION;
        ComPtr<IAppxManifestQualifiedResourcesEnumerator> loadedResources;
        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> loadedTdfs;

        ComPtr<IAppxManifestReader> manifestReader;
        ThrowHrIfFailed(packageReader->GetManifest(&manifestReader));
        ThrowHrIfFailed(manifestReader->GetPackageId(&loadedPackageId));

        ComPtr<IAppxManifestReader3> manifestReader3;
        ThrowHrIfFailed(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));
        ThrowHrIfFailed(manifestReader3->GetQualifiedResources(&loadedResources));
        ThrowHrIfFailed(manifestReader3->GetTargetDeviceFamilies(&loadedTdfs));

        auto packageIdInternal = loadedPackageId.As<IAppxManifestPackageIdInternal>();

        loadedPackageType = this->m_validationHelper.GetPayloadPackageType(manifestReader.Get(), fileName);
        this->m_validationHelper.AddPackage(loadedPackageType, packageIdInternal.Get(), fileName);
        this->m_validationHelper.ValidateOSVersion(manifestReader.Get(), fileName);        

        ValidateNameAndPublisher(packageIdInternal.Get(), fileName);

        if (loadedPackageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            this->m_validationHelper.ValidateApplicationElement(manifestReader.Get(), fileName);
            if (loadedTdfs.Get() != nullptr)
            {
                ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfCopy;
                ThrowHrIfFailed(manifestReader3->GetTargetDeviceFamilies(&tdfCopy));
                this->m_validationHelper.ValidateTargetDeviceFamiliesFromManifestPackageId(packageIdInternal.Get(), tdfCopy.Get(), fileName);
            }
        }

        *packageType = loadedPackageType;
        *packageId = loadedPackageId.Detach();
        *resources = loadedResources.Detach();

        if (loadedTdfs.Get() != nullptr)
        {
            *tdfs = loadedTdfs.Detach();
        }
    }

    void BundleWriterHelper::ValidateNameAndPublisher(IAppxManifestPackageIdInternal* packageId,
        std::string filename)
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
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because it has a different package family name than other packages in the bundle.");
            }

            std::string publisherName = packageId->GetPublisher();
            if ((this->mainPackagePublisher.compare(publisherName)) != 0)
            {
                std::string packageFullName = packageId->GetPackageFullName();
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because it has a different package family name than other packages in the bundle.");
            }
        }
    }

    void BundleWriterHelper::AddValidatedPackageData(
        std::string fileName, 
        std::uint64_t bundleOffset,
        std::uint64_t packageSize,
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType,
        ComPtr<IAppxManifestPackageId> packageId,
        bool isDefaultApplicablePackage,
        IAppxManifestQualifiedResourcesEnumerator* resources,
        IAppxManifestTargetDeviceFamiliesEnumerator* tdfs)
    {
        auto innerPackageIdInternal = packageId.As<IAppxManifestPackageIdInternal>();

        PackageInfo packageInfo;
        packageInfo.type = packageType;
        packageInfo.architecture = innerPackageIdInternal->GetArchitecture();
        UINT64 version;
        ThrowHrIfFailed(packageId->GetVersion(&version));
        packageInfo.version = version;
        packageInfo.resourceId = innerPackageIdInternal->GetResourceId();
        packageInfo.isDefaultApplicablePackage = isDefaultApplicablePackage;
        packageInfo.resources = resources;
        packageInfo.fileName = fileName;
        packageInfo.size = packageSize;
        packageInfo.offset = bundleOffset;
        packageInfo.tdfs = tdfs;

        AddPackageInfoToVector(this->payloadPackages, packageInfo);
    }

    void BundleWriterHelper::AddPackageInfoToVector(std::vector<PackageInfo>& packagesVector, 
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
    }

    void BundleWriterHelper::AddExternalPackageReferenceFromManifest(std::string fileName, IAppxManifestReader* manifestReader,
        bool isDefaultApplicablePackage)
    {
        //TODO: ValidatePayloadPackageExtension

        ComPtr<IAppxManifestReader5> manifestReader5;
        ThrowHrIfFailed(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader5)));
        ComPtr<IAppxManifestMainPackageDependenciesEnumerator> mainPackageDependencies;
        ThrowHrIfFailed(manifestReader5->GetMainPackageDependencies(&mainPackageDependencies));
        BOOL hasMoreMainPackageDependencies = FALSE;
        ThrowHrIfFailed(mainPackageDependencies->GetHasCurrent(&hasMoreMainPackageDependencies));

        // Validation: this must be an optional package with a main package dependency with the same
        // name and publisher as the actual main packages added to the bundle
        ThrowErrorIfNot(Error::AppxManifestSemanticError, hasMoreMainPackageDependencies, "The Appx package's manifest is invalid.");
        ComPtr<IAppxManifestMainPackageDependency> mainPackage;
        ThrowHrIfFailed(mainPackageDependencies->GetCurrent(&mainPackage));

        auto mainPackageInternal = mainPackage.As<IAppxManifestMainPackageDependencyInternal>();

        if(this->mainPackageName.empty())
        {
            this->mainPackageName = mainPackageInternal->GetName();
            this->mainPackagePublisher = mainPackageInternal->GetPublisher();
        }
        else
        {
            std::string packageName = mainPackageInternal->GetName();
            if(packageName.compare(this->mainPackageName) != 0)
            {
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The Appx package's manifest is invalid.");
            }
            
            std::string publisher = mainPackageInternal->GetPublisher();
            if(publisher.compare(this->mainPackagePublisher) != 0)
            {
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The Appx package's manifest is invalid.");
            }
        }

        // Validation: this cannot be an optional package with multiple main package dependencies
        ThrowHrIfFailed(mainPackageDependencies->MoveNext(&hasMoreMainPackageDependencies));
        ThrowErrorIf(Error::AppxManifestSemanticError, hasMoreMainPackageDependencies, "The Appx package's manifest is invalid.");

        // Populate a new PackageInfo entry with info from the optional package's manifest
        ComPtr<IAppxManifestPackageId> packageId1;
        ThrowHrIfFailed(manifestReader->GetPackageId(&packageId1));

        auto packageId = packageId1.As<IAppxManifestPackageIdInternal>();

        std::string bundleFamilyName = packageId->GetPackageFamilyName();
        std::map<std::string, OptionalBundleInfo>::iterator optBundlesIterator = optionalBundles.find(bundleFamilyName);
        if(optBundlesIterator == optionalBundles.end())
        {
            OptionalBundleInfo newBundleInfo;
            newBundleInfo.name = packageId->GetName();
            newBundleInfo.publisher = packageId->GetPublisher();
            newBundleInfo.version = 0;
            newBundleInfo.fileName = nullptr;

            optionalBundles.insert(std::pair<std::string, OptionalBundleInfo>(bundleFamilyName, newBundleInfo));
            optBundlesIterator->second = newBundleInfo;
        }

        PackageInfo packageInfo;
        packageInfo.type = this->m_validationHelper.GetPayloadPackageType(manifestReader, fileName);
        //BundleValidationHelper::AddPackage
        if (packageInfo.type == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            this->m_validationHelper.ValidateApplicationElement(manifestReader, fileName);
            //manifestComparisonHelper.AddManifest
        }

        packageInfo.architecture = packageId->GetArchitecture();
        UINT64 version;
        ThrowHrIfFailed(packageId1->GetVersion(&version));
        packageInfo.version = version;
        packageInfo.resourceId = packageId->GetResourceId();

        ComPtr<IAppxManifestReader2> manifestReader2;
        ThrowHrIfFailed(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader2>::iid, reinterpret_cast<void**>(&manifestReader2)));

        packageInfo.isDefaultApplicablePackage = isDefaultApplicablePackage;
        ThrowHrIfFailed(manifestReader2->GetQualifiedResources(&packageInfo.resources));
        packageInfo.fileName = fileName;
        packageInfo.offset = 0;
        packageInfo.size = 0;

        ComPtr<IAppxManifestReader3> manifestReader3;
        ThrowHrIfFailed(manifestReader2->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));
        manifestReader3->GetTargetDeviceFamilies(&packageInfo.tdfs);

        AddPackageInfoToVector(optBundlesIterator->second.optionalPackages, packageInfo);        
    }

    void BundleWriterHelper::EndBundleManifest()
    {        
        // A bundle must contain at least one app package.  It's an error to Close
        // the writer without having added one.
        bool result = this->m_validationHelper.ContainsApplicationPackage();
        if (!result)
        {
            ThrowErrorAndLog(Error::AppxManifestSemanticError, "The bundle must contain at least one app package targeting a known processor architecture.");
        }
        
        std::string targetXmlNamespace = "http://schemas.microsoft.com/appx/2013/bundle";
        bool isPre2018BundleManifest = true;
        
        // Only use new 2018 bundle schema if the bundle contains more than 1 neutral app packages
        if (this->m_validationHelper.ContainsMultipleNeutralAppPackages())
        {
            targetXmlNamespace = "http://schemas.microsoft.com/appx/2018/bundle";
            isPre2018BundleManifest = false;
        }
        else if (this->hasDefaultOrNeutralResources)
        {
            targetXmlNamespace = "http://schemas.microsoft.com/appx/2017/bundle";
        }
        else if ((this->optionalBundles.size() > 0) || this->hasExternalPackages)
        {
            targetXmlNamespace = "http://schemas.microsoft.com/appx/2016/bundle";
        }

        this->m_validationHelper.ValidateContainsMultipleNeutralAppPackages(isPre2018BundleManifest);
        m_bundleManifestWriter.StartBundleManifest(targetXmlNamespace, this->mainPackageName,
            this->mainPackagePublisher, this->bundleVersion);

        for(std::size_t i = 0; i < this->payloadPackages.size(); i++) 
        {
            m_bundleManifestWriter.AddPackage(payloadPackages[i]);
        }

        std::map<std::string, OptionalBundleInfo>::iterator optionalBundleIterator = optionalBundles.begin();     
        while(optionalBundleIterator != optionalBundles.end())
        {
            m_bundleManifestWriter.AddOptionalBundle(optionalBundleIterator->second);
            optionalBundleIterator++;
        }
        
        m_bundleManifestWriter.EndBundleManifest();
    }       
}