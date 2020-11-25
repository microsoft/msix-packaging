//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "XmlWriter.hpp"
#include "BundleManifestWriter.hpp"
#include "Crypto.hpp"
#include "StringHelper.hpp"
#include "AppxManifestObject.hpp"

#include <vector>

namespace MSIX {

    static const char* bundleManifestElement = "Bundle";
    static const char* schemaVersionAttribute = "SchemaVersion";
    static const char* Win2019SchemaVersion = "5.0";
    static const char* identityManifestElement = "Identity";
    static const char* nameAttribute = "Name";
    static const char* publisherAttribute = "Publisher";
    static const char* versionAttribute = "Version";
    static const char* packagesManifestElement = "Packages";
    static const char* packageManifestElement = "Package";
    static const char* packageTypeAttribute = "Type";
    static const char* packageArchitectureAttribute = "Architecture";
    static const char* packageResourceIdAttribute = "ResourceId";
    static const char* fileNameAttribute = "FileName";
    static const char* resourcesManifestElement = "Resources";
    static const char* resourceManifestElement = "Resource";
    static const char* resourceLanguageAttribute = "Language";
    static const char* dependenciesManifestElementWithoutPrefix = "Dependencies";
    static const char* targetDeviceFamilyManifestElementWithoutPrefix = "TargetDeviceFamily";
    static const char* tdfMinVersionAttribute = "MinVersion";
    static const char* tdfMaxVersionTestedAttribute = "MaxVersionTested";
    static const char* ApplicationPackageType = "application";
    static const char* ResourcePackageType = "resource";
    static const char* optionalBundleManifestElement = "OptionalBundle";

    static const char* NamespaceAlias = "b";
    static const char* Namespace = "http://schemas.microsoft.com/appx/2013/bundle";
    static const char* Namespace2016Alias = "b2";
    static const char* Namespace2016 = "http://schemas.microsoft.com/appx/2016/bundle";
    static const char* Namespace2017Alias = "b3";
    static const char* Namespace2017 = "http://schemas.microsoft.com/appx/2017/bundle";
    static const char* Namespace2018Alias = "b4";
    static const char* Namespace2018 = "http://schemas.microsoft.com/appx/2018/bundle";
    static const char* Namespace2019Alias = "b5";
    static const char* Namespace2019 = "http://schemas.microsoft.com/appx/2019/bundle";

    BundleManifestWriter::BundleManifestWriter() : m_xmlWriter(XmlWriter(bundleManifestElement)) 
    {
        currentState = Uninitialized;
    }

    void BundleManifestWriter::StartBundleManifest(std::string targetXmlNamespace, std::string name, 
        std::string publisher, std::uint64_t version)
    {
        this->targetXmlNamespace = targetXmlNamespace;
        StartBundleElement();
        WriteIdentityElement(name, publisher, version);
        StartPackagesElement();
        currentState = BundleManifestStarted;
    }

    void BundleManifestWriter::StartBundleElement()
    {
        m_xmlWriter.AddAttribute(xmlnsAttribute, this->targetXmlNamespace);
        m_xmlWriter.AddAttribute(schemaVersionAttribute, Win2019SchemaVersion);

        std::string bundle2018QName = GetQualifiedName(xmlnsAttribute, Namespace2018Alias);
        m_xmlWriter.AddAttribute(bundle2018QName, Namespace2018);

        std::string bundle2019QName = GetQualifiedName(xmlnsAttribute, Namespace2019Alias);
        m_xmlWriter.AddAttribute(bundle2019QName, Namespace2019);

        std::string ignorableNamespaces;
        ignorableNamespaces.append(Namespace2018Alias);
        ignorableNamespaces.append(" ");
        ignorableNamespaces.append(Namespace2019Alias);
        m_xmlWriter.AddAttribute("IgnorableNamespaces", ignorableNamespaces);
    }

    void BundleManifestWriter::WriteIdentityElement(std::string name, std::string publisher, std::uint64_t version)
    {
        m_xmlWriter.StartElement(identityManifestElement);

        m_xmlWriter.AddAttribute(nameAttribute, name);
        m_xmlWriter.AddAttribute(publisherAttribute, publisher);

        std::string versionString = MSIX::ConvertVersionToString(version);
        m_xmlWriter.AddAttribute(versionAttribute, versionString);

        m_xmlWriter.CloseElement();
    }

    void BundleManifestWriter::StartPackagesElement()
    {
        m_xmlWriter.StartElement(packagesManifestElement);
    }

    void BundleManifestWriter::AddPackage(PackageInfo packageInfo)
    {
        WritePackageElement(packageInfo);
        currentState = PackagesAdded;
    }

    void BundleManifestWriter::WritePackageElement(PackageInfo packageInfo)
    {
        m_xmlWriter.StartElement(packageManifestElement);

        std::string packageTypeString;
        if(packageInfo.type == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            packageTypeString = ApplicationPackageType;
        }
        else if (packageInfo.type == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE)
        {
            packageTypeString = ResourcePackageType;
        }
         m_xmlWriter.AddAttribute(packageTypeAttribute, packageTypeString);

        std::string versionString = MSIX::ConvertVersionToString(packageInfo.version);
        m_xmlWriter.AddAttribute(versionAttribute, versionString);

        if(packageInfo.type == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            m_xmlWriter.AddAttribute(packageArchitectureAttribute, packageInfo.architecture);
        }

        if (!packageInfo.resourceId.empty() && (packageInfo.resourceId.size() > 0))
        {
            m_xmlWriter.AddAttribute(packageResourceIdAttribute, packageInfo.resourceId);
        }

        if(!packageInfo.fileName.empty())
        {
            m_xmlWriter.AddAttribute(fileNameAttribute, packageInfo.fileName);
        }

        if(packageInfo.offset > 0)
        {
            //TODO: not applicable for flat bundle
        }

        if (packageInfo.size > 0 && packageInfo.offset > 0)
        {
            //TODO: not applicable for flat bundles
        }

        //WriteResourcesElement
        WriteResourcesElement(packageInfo.resources.Get());

        //WriteDependenciesElement
        WriteDependenciesElement(packageInfo.tdfs.Get());

        //End Package Tag
        m_xmlWriter.CloseElement();
    }

    void BundleManifestWriter::WriteResourcesElement(IAppxManifestQualifiedResourcesEnumerator* resources)
    {
        BOOL hasResources = FALSE;
        ThrowHrIfFailed(resources->GetHasCurrent(&hasResources));

        if (hasResources)
        {
            //Start Resources element
            m_xmlWriter.StartElement(resourcesManifestElement);

            BOOL hasNext = FALSE;
            ThrowHrIfFailed(resources->GetHasCurrent(&hasNext));
            while (hasNext)
            {
                ComPtr<IAppxManifestQualifiedResource> resource;
                ThrowHrIfFailed(resources->GetCurrent(&resource));

                //Start Resource element
                m_xmlWriter.StartElement(resourceManifestElement);

                auto qualifiedResourceInternal = resource.As<IAppxManifestQualifiedResourceInternal>();
                std::string languageString = qualifiedResourceInternal->GetLanguage();
                if (!languageString.empty())
                {
                    m_xmlWriter.AddAttribute(resourceLanguageAttribute, languageString);
                }

                //TODO:: Write scale and dxfeaturelevel attributes

                //End Resource element
                m_xmlWriter.CloseElement();

                ThrowHrIfFailed(resources->MoveNext(&hasNext));
            }

            //End Resources element
            m_xmlWriter.CloseElement(); 
        }
    }

    void BundleManifestWriter::WriteDependenciesElement(IAppxManifestTargetDeviceFamiliesEnumerator* tdfs)
    {
        BOOL hasNext = FALSE;
        ThrowHrIfFailed(tdfs->GetHasCurrent(&hasNext));

        if (hasNext)
        {
            std::string dependencyQName = GetElementName(Namespace2018, Namespace2018Alias, dependenciesManifestElementWithoutPrefix);
            m_xmlWriter.StartElement(dependencyQName);

            while (hasNext)
            {
                ComPtr<IAppxManifestTargetDeviceFamily> tdf;
                ThrowHrIfFailed(tdfs->GetCurrent(&tdf));

                //Start TargetDeviceFamily manifest element
                std::string tdfQName = GetElementName(Namespace2018, Namespace2018Alias, targetDeviceFamilyManifestElementWithoutPrefix);
                m_xmlWriter.StartElement(tdfQName);

                auto targetDeviceFamilyInternal = tdf.As<IAppxManifestTargetDeviceFamilyInternal>();
                std::string name = targetDeviceFamilyInternal->GetName();
                m_xmlWriter.AddAttribute(nameAttribute, name);

                //Get minversion
                UINT64 minVersion;
                ThrowHrIfFailed(tdf->GetMinVersion(&minVersion));
                std::string minVerionString = MSIX::ConvertVersionToString(minVersion);
                m_xmlWriter.AddAttribute(tdfMinVersionAttribute, minVerionString);

                //Get maxversiontested
                UINT64 maxVersionTested;
                ThrowHrIfFailed(tdf->GetMaxVersionTested(&maxVersionTested));
                std::string maxVersionTestedString = MSIX::ConvertVersionToString(maxVersionTested);
                m_xmlWriter.AddAttribute(tdfMaxVersionTestedAttribute, maxVersionTestedString);

                //End TargetDeviceFamily manifest element
                m_xmlWriter.CloseElement();

                ThrowHrIfFailed(tdfs->MoveNext(&hasNext));
            }

            //End Dependencies Tag
            m_xmlWriter.CloseElement();
        }
    }

    void BundleManifestWriter::AddOptionalBundle(OptionalBundleInfo bundleInfo)
    {
        EndPackagesElementIfNecessary();
        WriteOptionalBundleElement(bundleInfo);
        currentState = OptionalBundlesAdded;
    }

    // Writes an OptionalBundle element, which can have one or more Package elements inside.
    // <OptionalBundle Name="opt2" Publisher="CN=DifferentPublisher" Version="1.0.0.0" FileName="OptionalBundle3.appxbundle">
    void BundleManifestWriter::WriteOptionalBundleElement(OptionalBundleInfo bundleInfo)
    {
        ThrowErrorIf(Error::InvalidParameter, bundleInfo.name.empty(), "One or more arguments are invalid");
        ThrowErrorIf(Error::InvalidParameter, bundleInfo.publisher.empty(), "One or more arguments are invalid");

        m_xmlWriter.StartElement(optionalBundleManifestElement);
        m_xmlWriter.AddAttribute(nameAttribute, bundleInfo.name);
        m_xmlWriter.AddAttribute(publisherAttribute, bundleInfo.publisher);

        if (bundleInfo.version > 0)
        {
            std::string versionString = MSIX::ConvertVersionToString(bundleInfo.version);
            m_xmlWriter.AddAttribute(versionAttribute, versionString);
        }

        if(!bundleInfo.fileName.empty())
        {
            m_xmlWriter.AddAttribute(fileNameAttribute, bundleInfo.fileName);
        }

        for(size_t i = 0; i < bundleInfo.optionalPackages.size(); i++)
        {
            WritePackageElement(bundleInfo.optionalPackages[i]);
        }

        //End OptionalBundle Tag
        m_xmlWriter.CloseElement();
    }

    void BundleManifestWriter::EndBundleManifest()
    {
        EndPackagesElementIfNecessary();
        EndBundleElement();
        currentState = BundleManifestEnded;
    }

    void BundleManifestWriter::EndPackagesElementIfNecessary()
    {
        if (currentState == PackagesAdded)
        {
            EndPackagesElement();
        }
    }

    void BundleManifestWriter::EndPackagesElement()
    {
        m_xmlWriter.CloseElement();
    }

    void BundleManifestWriter::EndBundleElement()
    {
        m_xmlWriter.CloseElement();
    }

    std::string BundleManifestWriter::GetElementName(std::string targetNamespace, std::string targetNamespaceAlias, std::string name)
    {
        std::string qualifiedName;
        if ((this->targetXmlNamespace.compare(targetNamespace) != 0) && (!targetNamespaceAlias.empty()))
        {
            qualifiedName = GetQualifiedName(targetNamespaceAlias, name);
        }
        else
        {
            qualifiedName = name;
        }
        return qualifiedName;
    }

    std::string BundleManifestWriter::GetQualifiedName(std::string namespaceAlias, std::string name)
    {
        std::string output;
        output.append(namespaceAlias);
        output.append(xmlNamespaceDelimiter);
        output.append(name);
        return output;
    }
}

