//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"
#include "Exceptions.hpp"
#include "UnicodeConversion.hpp"
#include "GeneralUtil.hpp"

#include <vector>

namespace MSIX {

    struct PackageInfo
    {
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
        std::uint64_t version;
        std::string architecture;
        std::string resourceId;
        std::string fileName;
        std::uint64_t size;
        std::uint64_t offset;
        //ComPtr<IAppxManifestQualifiedResourcesEnumerator> resources;
        ComPtr<IAppxManifestResourcesEnumerator> resources;
        BOOL isDefaultApplicablePackage;
        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfs;
    };

    class BundleManifestWriter final
    {
    public:
        BundleManifestWriter();
        void BundleManifestWriter::StartBundleManifest(std::string targetXmlNamespace, 
            std::string name, std::string publisher, UINT64 version);
        void BundleManifestWriter::StartBundleElement();
        void BundleManifestWriter::WriteIdentityElement(std::string name, std::string publisher, UINT64 version);
        void BundleManifestWriter::StartPackagesElement();
        HRESULT BundleManifestWriter::WritePackageElement(PackageInfo packageInfo);
        //HRESULT BundleManifestWriter::WriteResourcesElement(IAppxManifestResourcesEnumerator* resources);
        HRESULT BundleManifestWriter::WriteDependenciesElement(IAppxManifestTargetDeviceFamiliesEnumerator* tdfs);
        void BundleManifestWriter::EndPackagesElement();
        void BundleManifestWriter::Close();

        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }
        std::string GetQualifiedName(std::string namespaceAlias);

    protected:
        XmlWriter m_xmlWriter;
        std::string targetXmlNamespace;

    };
}