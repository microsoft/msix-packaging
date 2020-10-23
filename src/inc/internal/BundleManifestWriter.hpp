//
//  Copyright (C) 2019 Microsoft.  All rights reserved.
//  See LICENSE file in the project root for full license information.
// 
#pragma once

#include "XmlWriter.hpp"
#include "ComHelper.hpp"

#include <vector>

namespace MSIX {

    class BundleManifestWriter final
    {
    public:
        BundleManifestWriter();
        void CreateBundleElement(std::string targetXmlNamespace);
        /*HRESULT StartBundleManifest(std::string bundleName, std::string bundlePublisher, UINT64 bundleVersion);
        HRESULT AddPackage(/*PackageInfo* packageInfo);
        HRESULT AddOptionalBundle(/*OptionalBundleInfo* bundleInfo);
        HRESULT EndBundleManifest();
        HRESULT InitializeWriter();*/

        ComPtr<IStream> GetStream() { return m_xmlWriter.GetStream(); }
        std::string GetQualifiedName(std::string namespaceAlias);

    protected:
        XmlWriter m_xmlWriter;
    };
}