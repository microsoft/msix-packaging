//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <vector>

namespace MSIX {

    enum ElementWriterState
    {
        Uninitialized = 0,
        BundleManifestStarted = 1,
        PackagesAdded = 2,
        OptionalBundlesAdded = 3,
        BundleManifestEnded = 4,
    };

    class BundleManifestWriter final
    {
    public:
        BundleManifestWriter();
        void BundleManifestWriter::StartBundleManifest(std::string targetXmlNamespace, 
            std::string name, std::string publisher, UINT64 version);
        void BundleManifestWriter::StartBundleElement(std::string targetXmlNamespace);
        void BundleManifestWriter::WriteIdentityElement(std::string name, std::string publisher, UINT64 version);
        void BundleManifestWriter::StartPackagesElement();
        void BundleManifestWriter::WritePackageElement(APPX_BUNDLE_PAYLOAD_PACKAGE_TYPE packageType, 
            UINT64 version, std::string architecture, std::string resourceId, std::string fileName, UINT64 offset);
        void BundleManifestWriter::EndBundleManifest();

        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }
        std::string GetQualifiedName(std::string namespaceAlias);
        std::string ConvertVersionToString(UINT64 version);

    protected:
        XmlWriter m_xmlWriter;
        bool packageAdded = false;

        //std::string targetXmlNamespace;
        //UINT32 currentState;

    };
}