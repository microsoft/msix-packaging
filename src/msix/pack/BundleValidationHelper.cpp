#include "BundleValidationHelper.hpp"

namespace MSIX {

    const std::string Win10DeviceFamilyNameStartsWith = "Windows.";
    const std::string OSVersionRS5 = "10.0.17763.0";

    BundleValidationHelper::BundleValidationHelper() 
    {}

    void BundleValidationHelper::ValidateTargetDeviceFamiliesFromManifestPackageId(IAppxManifestPackageIdInternal* packageId, IAppxManifestTargetDeviceFamiliesEnumerator* tdfs, std::string fileName)
    {
        std::string arch = packageId->GetArchitecture();
        if (arch == "neutral")
        {
            this->numberOfNeutralAppPackages++;
        }

        BOOL hasCurrent = FALSE;
        ThrowHrIfFailed(tdfs->GetHasCurrent(&hasCurrent));
        while (hasCurrent)
        {
            ComPtr<IAppxManifestTargetDeviceFamily> tdf;
            ThrowHrIfFailed(tdfs->GetCurrent(&tdf));

            auto tdfInternal = tdf.As<IAppxManifestTargetDeviceFamilyInternal>();
            std::string tdfName = tdfInternal->GetName();

            if(!this->bundleTargetsRs5OrLess && StartsWith(tdfName, Win10DeviceFamilyNameStartsWith))
            {
                std::uint64_t minVersion;
                ThrowHrIfFailed(tdf->GetMinVersion(&minVersion));
                if (minVersion <= ConvertVersionStringToUint64(OSVersionRS5))
                {
                    this->bundleTargetsRs5OrLess = true;
                }
            }

            if(arch == "neutral")
            {
                PackageNameInfo packageNameInfo;
                packageNameInfo.packageFullName = packageId->GetPackageFullName();
                packageNameInfo.fileName = fileName;

                if(!(this->neutralTdfs.insert(std::make_pair(tdfName, packageNameInfo)).second))
                {
                    std::map<std::string, PackageNameInfo>::iterator neutralMapIterator = this->neutralTdfs.find(tdfName);
                    if(neutralMapIterator != this->neutralTdfs.end())
                    {
                        std::string foundFileName = neutralMapIterator->second.fileName;
                        std::string foundFullName = neutralMapIterator->second.packageFullName;

                        std::ostringstream errorBuilder;
                        errorBuilder << "The package with file name " << packageNameInfo.fileName << " and package full name " << 
                            packageNameInfo.packageFullName << " is not valid in the bundle because the bundle also contains the package with file name " << foundFileName << " and package full name " << foundFullName << " which targets the same device family. Bundles cannot contain multiple neutral app packages with the same target device family value.";
                        ThrowErrorAndLog(Error::AppxManifestSemanticError, errorBuilder.str().c_str());
                    }
                }
            }
            ThrowHrIfFailed(tdfs->MoveNext(&hasCurrent));
        }
    }

    bool BundleValidationHelper::StartsWith(std::string str, std::string prefix)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
        return ((prefix.size() <= str.size()) && std::equal(prefix.begin(), prefix.end(), str.begin()));
    }

    bool BundleValidationHelper::ContainsMultipleNeutralAppPackages()
    {
        return (numberOfNeutralAppPackages > 1);
    }

    // Checks if an application package has been added to the bundle.
    bool BundleValidationHelper::ContainsApplicationPackage()
    {
        return this->containsApplicationPackage;
    }

    void BundleValidationHelper::ValidateContainsMultipleNeutralAppPackages(bool isPre2018BundleManifest)
    {
        if ((this->numberOfNeutralAppPackages > 1) && (this->bundleTargetsRs5OrLess || isPre2018BundleManifest))
        {
            // Pre 19H1 bundles with multiple neutral app packages are invalid.
            // Bundles with multiple neutral app packages that use the 19H1 schema are invalid if any of their
            // packages targets RS5 or lower.
            ThrowErrorAndLog(Error::AppxManifestSemanticError, "The bundle contains at least two conflicting app packages. Bundles that contain packages targeting 10.0.17763.0 or lower cannot contain more than one neutral application package.");
        }
    }

    // Tests if adding a new package to the bundle is still valid.
    // Checks if the new package can be added to the bundle without breaking any
    // semantic constraints. AppxManifestSemanticError if its architecture, resource
    // ID, or package name conflicts with another package already added to this
    // validation helper.
    void BundleValidationHelper::AddPackage(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type, IAppxManifestPackageIdInternal* packageId, std::string fileName)
    {
        if (this->numberOfPackagesAdded >= MaxNumberOfPackages)
        {
            ThrowErrorAndLog(Error::OutOfBounds, "The bundle cannot contain more than 10000 packages.");
        }

        PackageNameInfo packageNameInfo;
        packageNameInfo.packageFullName = packageId->GetPackageFullName();
        packageNameInfo.fileName = fileName;

        if(!fileNamesMap.empty())
        {
            if(!(this->fileNamesMap.insert(std::make_pair(fileName, packageNameInfo)).second))
            {
                std::ostringstream errorBuilder;
                errorBuilder << "The package with file name " << packageNameInfo.fileName << " and package full name " << packageNameInfo.packageFullName << " is not valid in the bundle because the bundle contains another package with the same file name.";
                ThrowErrorAndLog(Error::AppxManifestSemanticError, errorBuilder.str().c_str());
            }
        }

        if (type == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            this->containsApplicationPackage = true;
        }
        else if (type == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE)
        {
            ValidateResourcePackage(packageId, packageNameInfo);
        }
        else
        {
            // unknown package type
            ThrowErrorAndLog(Error::PackagingErrorInternal, "MSIX packaging API has encountered an internal error.");
        }

        this->numberOfPackagesAdded++;
    }

    void BundleValidationHelper::ValidateResourcePackage(IAppxManifestPackageIdInternal* packageId, PackageNameInfo packageNameInfo)
    {
        std::string resourceId = packageId->GetResourceId();
        if(resourceId.empty())
        {
            // Resource ID is an empty string by default if it's not declared
            resourceId = "";
        }

        // The Insert method fails if the entry is already found in the map.
        // In that case we have a duplicate resource ID.
        if(!(this->resourceIds.insert(std::make_pair(resourceId, packageNameInfo)).second))
        {
            std::map<std::string, PackageNameInfo>::iterator resourceIdMapIterator = this->resourceIds.find(resourceId);
            if(resourceIdMapIterator != this->resourceIds.end())
            {
                std::string foundFileName = resourceIdMapIterator->second.fileName;
                std::string foundFullName = resourceIdMapIterator->second.packageFullName;

                std::ostringstream errorBuilder;
                errorBuilder << "The package with file name " << packageNameInfo.fileName << " and package full name " << 
                    packageNameInfo.packageFullName << " is not valid in the bundle because the bundle also contains the package with file name " << foundFileName << " and package full name " << foundFullName << " which has the same resource ID. Bundles cannot contain multiple resource packages with the same resource ID.";
                ThrowErrorAndLog(Error::AppxManifestSemanticError, errorBuilder.str().c_str());
            }
        }
    }

    // Reads the <Properties> element of a package manifest and determines the type
    // (either Application or Resource) of the package.  This method will fail with
    // AppxManifestSemanticError if the manifest is for a Framework package.
    APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE BundleValidationHelper::GetPayloadPackageType(IAppxManifestReader* packageManifestReader, std::string fileName)
    {
        ComPtr<IAppxManifestProperties> packageProperties;
        ThrowHrIfFailed(packageManifestReader->GetProperties(&packageProperties));

        BOOL isFrameworkPackage = FALSE;
        ThrowHrIfFailed(packageProperties->GetBoolValue(L"Framework", &isFrameworkPackage));

        if (isFrameworkPackage)
        {
            ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because it is a framework package.");
        }

        BOOL isResourcePackage = FALSE;
        ThrowHrIfFailed(packageProperties->GetBoolValue(L"ResourcePackage", &isResourcePackage));

        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType = (isResourcePackage ? APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE : APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION);
        return packageType;
    }

    // Validates that the package manifest declares OSMinVersion and OSMaxVersionTested values that
    // are at least MinimumAllowedOSVersion (6.3).
    void BundleValidationHelper::ValidateOSVersion(IAppxManifestReader* packageManifestReader, std::string fileName)
    {
        ComPtr<IAppxManifestReader> manifestReader(packageManifestReader);
        ComPtr<IAppxManifestReader3> manifestReader3;
        ThrowHrIfFailed(manifestReader->QueryInterface(UuidOfImpl<IAppxManifestReader3>::iid, reinterpret_cast<void**>(&manifestReader3)));

        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> targetDeviceFamilies;
        if (SUCCEEDED(manifestReader3->GetTargetDeviceFamilies(&targetDeviceFamilies)))
        {
            // Manifests that declare target device families are for Threshold and later.
            // This meets the OS version requirement
            return;
        }
        else
        {
            const LPCWSTR ElementsToTest[] = {L"OSMinVersion", L"OSMaxVersionTested"};
            for (int i = 0; i < ARRAYSIZE(ElementsToTest); i++)
            {
                std::uint64_t elementValue = 0;
                ThrowHrIfFailed(packageManifestReader->GetPrerequisite(ElementsToTest[i], &elementValue));
                if (elementValue < ConvertVersionStringToUint64(MinimumAllowedOSVersion))
                {
                    ComPtr<IAppxManifestPackageId> packageId;
                    ThrowHrIfFailed(packageManifestReader->GetPackageId(&packageId));
                    auto packageIdInternal = packageId.As<IAppxManifestPackageIdInternal>();
                    std::string packageFullName = packageIdInternal->GetPackageFullName();
                    std::ostringstream errorBuilder;
                        errorBuilder << "The package with file name " << fileName << " and package full name " << packageFullName << " is not valid in the bundle because its manifest declares a value for " 
                            << ElementsToTest[i] << " which is not supported in bundles. The minimum supported value is " << MinimumAllowedOSVersion << ".";
                    ThrowErrorAndLog(Error::AppxManifestSemanticError, errorBuilder.str().c_str());
                }
            }
        }
    }

    // Validates that the package manifest declares at least one <Application> element.
    void BundleValidationHelper::ValidateApplicationElement(IAppxManifestReader* packageManifestReader, std::string fileName)
    {
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
                ThrowErrorAndLog(Error::AppxManifestSemanticError, "The package is not valid in the bundle because its manifest does not declare any Application elements.");
            }
        }
    }
}