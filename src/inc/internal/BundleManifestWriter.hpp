//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"
#include "Exceptions.hpp"
#include "UnicodeConversion.hpp"
#include "VersionHelpers.hpp"

#include <vector>

namespace MSIX {

    // Tracks the internal state of the Bundle Manifest writer
    enum ElementWriterState
    {
        Uninitialized = 0,
        BundleManifestStarted = 1,
        PackagesAdded = 2,
        OptionalBundlesAdded = 3,
        BundleManifestEnded = 4,
    };

    struct PackageInfo
    {
        APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE type;
        std::uint64_t version;
        std::string architecture;
        std::string resourceId;
        std::string fileName;
        std::uint64_t size;
        std::uint64_t offset;
        ComPtr<IAppxManifestQualifiedResourcesEnumerator> resources;
        bool isDefaultApplicablePackage;
        ComPtr<IAppxManifestTargetDeviceFamiliesEnumerator> tdfs;
    };

    struct OptionalBundleInfo
    {
        std::string name;
        std::string publisher;
        std::uint64_t version;
        std::string fileName;
        std::vector<PackageInfo> optionalPackages;
    };

    class BundleManifestWriter final
    {
    public:
        BundleManifestWriter();
        void StartBundleManifest(std::string targetXmlNamespace, 
            std::string name, std::string publisher, std::uint64_t version);
        void StartBundleElement();
        void WriteIdentityElement(std::string name, std::string publisher, std::uint64_t version);
        void StartPackagesElement();
        void AddPackage(PackageInfo packageInfo);
        void WritePackageElement(PackageInfo packageInfo);
        void WriteResourcesElement(IAppxManifestQualifiedResourcesEnumerator* resources);
        void WriteDependenciesElement(IAppxManifestTargetDeviceFamiliesEnumerator* tdfs);
        void AddOptionalBundle(OptionalBundleInfo bundleInfo);
        void WriteOptionalBundleElement(OptionalBundleInfo bundleInfo);
        void EndPackagesElementIfNecessary();
        void EndPackagesElement();
        void EndBundleManifest();
        void EndBundleElement();

        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }
        std::string GetQualifiedName(std::string namespaceAlias, std::string name);
        std::string GetElementName(std::string targetNamespace, std::string targetNamespaceAlias, std::string name);

    protected:
        XmlWriter m_xmlWriter;
        std::string targetXmlNamespace;
        std::uint32_t currentState;
    };
}