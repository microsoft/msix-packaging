//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 

#include "XmlWriter.hpp"
#include "BundleManifestWriter.hpp"
#include "Crypto.hpp"
#include "StringHelper.hpp"

#include <vector>

namespace MSIX {

    static const char* bundleManifestElement = "Bundle";
    static const char* schemaVersionAttribute = "SchemaVersion";
    static const char* Win2019SchemaVersion = "5.0";
    static const char* identityManifestElement = "Identity";
    static const char* identityNameAttribute = "Name";
    static const char* identityPublisherAttribute = "Publisher";
    static const char* identityVersionAttribute = "Version";
    static const char* packagesManifestElement = "Packages";
    static const char* packageManifestElement = "Package";
    static const char* packageTypeAttribute = "Type";
    static const char* packageVersionAttribute = "Version";
    static const char* packageArchitectureAttribute = "Architecture";
    static const char* packageResourceIdAttribute = "ResourceId";
    static const char* packageFileNameAttribute = "FileName";
    static const char* resourcesManifestElement = "Resources";
    static const char* resourceManifestElement = "Resource";
    static const char* resourceLanguageAttribute = "Language";
    static const char* dependenciesManifestElement = "Dependencies";
    static const char* targetDeviceFamilyManifestElement = "TargetDeviceFamily";
    static const char* tdfNameAttribute = "Name";
    static const char* tdfMinVersionAttribute = "MinVersion";
    static const char* tdfMaxVersionTestedAttribute = "MaxVersionTested";
                    
    static const char* ApplicationPackageType = "application";
    static const char* ResourcePackageType = "resource";

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

    BundleManifestWriter::BundleManifestWriter() : m_xmlWriter(XmlWriter(bundleManifestElement)) {}

    void BundleManifestWriter::StartBundleManifest(std::string targetXmlNamespace, std::string name, 
        std::string publisher, UINT64 version)
    {
        this->targetXmlNamespace = targetXmlNamespace;
        StartBundleElement();
        WriteIdentityElement(name, publisher, version);
        StartPackagesElement();
    }

    void BundleManifestWriter::StartBundleElement()
    {
        m_xmlWriter.AddAttribute(xmlnsAttribute, this->targetXmlNamespace);
        m_xmlWriter.AddAttribute(schemaVersionAttribute, Win2019SchemaVersion);

        std::string bundle2018QName = GetQualifiedName(Namespace2018Alias);
        m_xmlWriter.AddAttribute(bundle2018QName, Namespace2018);

        std::string bundle2019QName = GetQualifiedName(Namespace2019Alias);
        m_xmlWriter.AddAttribute(bundle2019QName, Namespace2019);

        std::string ignorableNamespaces;
        ignorableNamespaces.append(Namespace2018Alias);
        ignorableNamespaces.append(" ");
        ignorableNamespaces.append(Namespace2019Alias);
        m_xmlWriter.AddAttribute("IgnorableNamespaces", ignorableNamespaces);
    }

    void BundleManifestWriter::WriteIdentityElement(std::string name, std::string publisher, UINT64 version)
    {
        m_xmlWriter.StartElement(identityManifestElement);

        m_xmlWriter.AddAttribute(identityNameAttribute, name);
        m_xmlWriter.AddAttribute(identityPublisherAttribute, publisher);

        std::string versionString = ConvertVersionToString(version);
        m_xmlWriter.AddAttribute(identityVersionAttribute, versionString);

        m_xmlWriter.CloseElement();
    }

    void BundleManifestWriter::StartPackagesElement()
    {
        m_xmlWriter.StartElement(packagesManifestElement);
    }

    HRESULT BundleManifestWriter::WritePackageElement(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType, 
        UINT64 version, std::string architecture, std::string resourceId, std::string fileName, UINT64 offset, 
        IAppxManifestQualifiedResourcesEnumerator* resources, IAppxManifestTargetDeviceFamiliesEnumerator* tdfs)
    {
        this->packageAdded = true;
        //if isStub, then Package tag is different
        m_xmlWriter.StartElement(packageManifestElement);

        std::string packageTypeString;
        if(packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            packageTypeString = ApplicationPackageType;
        }
        else if (packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_RESOURCE)
        {
            packageTypeString = ResourcePackageType;
        }
         m_xmlWriter.AddAttribute(packageTypeAttribute, packageTypeString);

        std::string versionString = ConvertVersionToString(version);
        m_xmlWriter.AddAttribute(packageVersionAttribute, versionString);

        if(packageType == APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE::APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE_APPLICATION)
        {
            m_xmlWriter.AddAttribute(packageArchitectureAttribute, architecture);
        }

        if (!resourceId.empty() && (resourceId.size() > 0))
        {
            m_xmlWriter.AddAttribute(packageResourceIdAttribute, resourceId);
        }

        if(!fileName.empty())
        {
            m_xmlWriter.AddAttribute(packageFileNameAttribute, fileName);
        }

        //WriteResourcesElement
        ThrowHrIfFailed(WriteResourcesElement(resources));

        //WriteDependenciesElement
        ThrowHrIfFailed(WriteDependenciesElement(tdfs));

        //End Package Tag
        m_xmlWriter.CloseElement();
        return S_OK;
    }

    HRESULT BundleManifestWriter::WriteResourcesElement(IAppxManifestQualifiedResourcesEnumerator* resources)
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

                LPWSTR languageString;
                ThrowHrIfFailed(resource->GetLanguage(&languageString));
                if (languageString != nullptr)
                {
                    m_xmlWriter.AddAttribute(resourceLanguageAttribute, wstring_to_utf8(languageString));
                }

                //End Resource element
                m_xmlWriter.CloseElement();

                ThrowHrIfFailed(resources->MoveNext(&hasNext));
            }

            //End Resources element
            m_xmlWriter.CloseElement(); 
        }
        return S_OK;
    }

    HRESULT BundleManifestWriter::WriteDependenciesElement(IAppxManifestTargetDeviceFamiliesEnumerator* tdfs)
    {
        BOOL hasNext = FALSE;
        ThrowHrIfFailed(tdfs->GetHasCurrent(&hasNext));

        if (hasNext)
        {
            m_xmlWriter.StartElement(dependenciesManifestElement);

            while (hasNext)
            {
                ComPtr<IAppxManifestTargetDeviceFamily> tdf;
                ThrowHrIfFailed(tdfs->GetCurrent(&tdf));

                //Start TargetDeviceFamily manifest element
                m_xmlWriter.StartElement(targetDeviceFamilyManifestElement);

                LPWSTR name;
                ThrowHrIfFailed(tdf->GetName(&name));
                m_xmlWriter.AddAttribute(tdfNameAttribute, wstring_to_utf8(name));

                UINT64 minVersion;
                ThrowHrIfFailed(tdf->GetMinVersion(&minVersion));
                std::string minVerionString = ConvertVersionToString(minVersion);
                m_xmlWriter.AddAttribute(tdfMinVersionAttribute, minVerionString);

                UINT64 maxVersionTested;
                ThrowHrIfFailed(tdf->GetMaxVersionTested(&maxVersionTested));
                std::string maxVersionTestedString = ConvertVersionToString(maxVersionTested);
                m_xmlWriter.AddAttribute(tdfMaxVersionTestedAttribute, maxVersionTestedString);

                //End TargetDeviceFamily manifest element
                m_xmlWriter.CloseElement();

                ThrowHrIfFailed(tdfs->MoveNext(&hasNext));
            }

            //End Dependencies Tag
            m_xmlWriter.CloseElement();
        }
        return S_OK;
    }

    void BundleManifestWriter::EndPackagesElement()
    {
        //if (currentState == PackagesAdded)
        //{
            //Ends Packages Element if a package has been added
            m_xmlWriter.CloseElement();
        //}
    }

    void BundleManifestWriter::Close()
    {
        //Ends Bundle Element
        m_xmlWriter.CloseElement();
    }

    std::string BundleManifestWriter::GetQualifiedName(std::string namespaceAlias)
    {
        std::string output;
        output.append(xmlnsAttribute);
        output.append(xmlNamespaceDelimiter);
        output.append(namespaceAlias);
        return output;
    }

    std::string BundleManifestWriter::ConvertVersionToString(UINT64 version)
    {
        return std::to_string((version >> 0x30) & 0xFFFF) + "."
            + std::to_string((version >> 0x20) & 0xFFFF) + "."
            + std::to_string((version >> 0x10) & 0xFFFF) + "."
            + std::to_string((version) & 0xFFFF);
    }


}

